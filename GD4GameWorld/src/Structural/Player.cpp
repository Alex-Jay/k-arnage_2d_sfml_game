#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Character.hpp"

#include <map>
#include <iostream>

using namespace std::placeholders;

struct CharacterMover
{
	sf::Vector2f velocity;
	float angle;
	unsigned int localIdentifier;

	CharacterMover(float vx, float vy, float da, unsigned int id) 
		: velocity(vx, vy)
		, angle(da)
		, localIdentifier(id)
	{
	}

	void operator()(Character& Character, sf::Time) const
	{
		if (Character.getLocalIdentifier() == localIdentifier)
		{
			Character.accelerate(velocity * Character.getMaxSpeed());
			Character.applyRotation(angle * Character.getMaxRotationSpeed());
		}
	}
};

struct CharacterFireTrigger
{
	unsigned int localIdentifier;

	CharacterFireTrigger(unsigned int id)
		: localIdentifier(id)
	{
	}

	void operator() (Character& Character, sf::Time) const
	{
		if (Character.getLocalIdentifier() == localIdentifier)
		{
			Character.fire();
		}
	}
};

struct CharacterGrenadeStarter
{
	unsigned int localIdentifier;

	CharacterGrenadeStarter(unsigned int id)
		: localIdentifier(id)
	{}

	void operator() (Character& Character, sf::Time) const
	{
		if (Character.getLocalIdentifier() == localIdentifier)
		{
			Character.startGrenade();
		}
	}
};

struct CharacterGrenadeLauncher
{
	unsigned int localIdentifier;

	CharacterGrenadeLauncher(unsigned int id)
		: localIdentifier(id)
	{}

	void operator() (Character& Character, sf::Time) const
	{
		if (Character.getLocalIdentifier() == localIdentifier)
		{
			Character.launchGrenade();
		}
	}
};

Player::Player(int localIdentifier)
	: mCurrentMissionStatus(MissionStatus::MissionRunning)
	, mJoystick(nullptr)
	, mLocalIdentifier(localIdentifier)
{
	// If joystick not set and there are available controllers
	if (mJoystick == nullptr && sf::Joystick::Count > 0)
	{
		// Setup controller with local ID
		setJoystick(new Xbox360Controller(localIdentifier));
	}
	else
	{
		// Else, reset the joystick if disconnected
		setJoystick(nullptr);
	}


	if (localIdentifier == 0)
	{
		//Set initial key bindings
		mKeyBindingPressed[sf::Keyboard::A] = Action::MoveLeft;
		mKeyBindingPressed[sf::Keyboard::D] = Action::MoveRight;
		mKeyBindingPressed[sf::Keyboard::W] = Action::MoveUp;
		mKeyBindingPressed[sf::Keyboard::S] = Action::MoveDown;
		mKeyBindingPressed[sf::Keyboard::Space] = Action::Fire;
		mKeyBindingPressed[sf::Keyboard::N] = Action::StartGrenade;
		mKeyBindingReleased[sf::Keyboard::N] = Action::LaunchGrenade;
		// Alex - Init. rotation keys -------------------------
		mKeyBindingPressed[sf::Keyboard::J] = Action::RotateLeft;
		mKeyBindingPressed[sf::Keyboard::K] = Action::RotateRight;
	}
	else if (localIdentifier == 1)
	{
		//Set initial key bindings
		mKeyBindingPressed[sf::Keyboard::Left] = Action::MoveLeft;
		mKeyBindingPressed[sf::Keyboard::Right] = Action::MoveRight;
		mKeyBindingPressed[sf::Keyboard::Up] = Action::MoveUp;
		mKeyBindingPressed[sf::Keyboard::Down] = Action::MoveDown;
		mKeyBindingPressed[sf::Keyboard::Numpad0] = Action::Fire;
		mKeyBindingPressed[sf::Keyboard::Enter] = Action::StartGrenade;
		mKeyBindingReleased[sf::Keyboard::Enter] = Action::LaunchGrenade;
		// Alex - Init. rotation keys -------------------------
		mKeyBindingPressed[sf::Keyboard::Numpad4] = Action::RotateLeft;
		mKeyBindingPressed[sf::Keyboard::Numpad6] = Action::RotateRight;
	}


	// Set intial joystick button bindings
	mJoystickBindingPressed[JoystickButton::RB] = Action::Fire;
	mJoystickBindingPressed[JoystickButton::X] = Action::StartGrenade;
	mJoystickBindingReleased[JoystickButton::X] = Action::LaunchGrenade;

	
	//set initial action bindings
	initializeActions();

	//Assign all categories to the player's Character
	for (auto& pair : mActionBinding)
	{
		pair.second.category = static_cast<unsigned int>(Category::PlayerCharacter);
	}
}

// Executes when button is pressed once
void Player::handleEvent(const sf::Event& event, CommandQueue& commands)
{
	

	if (mJoystick->IsConnected())
	{
		if (event.type == sf::Event::JoystickButtonPressed)
		{
			if (event.joystickButton.joystickId == getLocalIdentifier())
			{
				auto found = mJoystickBindingPressed.find(static_cast<JoystickButton>(event.joystickButton.button));

				if (found != mJoystickBindingPressed.end() && !isRealtimeAction(found->second))
				{
					commands.push(mActionBinding[found->second]);
				}
			}
		}

		if (event.type == sf::Event::JoystickButtonReleased)
		{

			if (event.joystickButton.joystickId == getLocalIdentifier())
			{
				auto found = mJoystickBindingReleased.find(static_cast<JoystickButton>(event.joystickButton.button));

				if (found != mJoystickBindingReleased.end() && !isRealtimeAction(found->second))
				{
					commands.push(mActionBinding[found->second]);
				}
			}
		}
	}

	//check if key pressed is in the key bindings, if so trigger command
	if (event.type == sf::Event::KeyPressed)
	{
		auto found = mKeyBindingPressed.find(event.key.code);

		if (found != mKeyBindingPressed.end() && !isRealtimeAction(found->second))
		{
			commands.push(mActionBinding[found->second]);
		}
	}

	//Mike
	if (event.type == sf::Event::KeyReleased)
	{
		auto found = mKeyBindingReleased.find(event.key.code);

		if (found != mKeyBindingReleased.end() && !isRealtimeAction(found->second))
		{
			commands.push(mActionBinding[found->second]);
		}
	}
}

// Executes while holding a button
void Player::handleRealtimeInput(CommandQueue& commands)
{
	for (auto pair : mKeyBindingPressed)
	{
		if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
		{
			commands.push(mActionBinding[pair.second]);
		}
	}

	// If joystick exists and is set in Player class
	if (mJoystick->IsConnected())
	{
		for (auto pair : mJoystickBindingPressed)
		{
			if (sf::Joystick::isButtonPressed(mLocalIdentifier, static_cast<unsigned int>(pair.first)) && isRealtimeAction(pair.second))
			{
				commands.push(mActionBinding[pair.second]);
			}
		}


		if (sf::Event::JoystickMoved)
		{
			if (mJoystick->Up())
			{
				commands.push(mActionBinding[Action::MoveUp]);
			}

			if (mJoystick->Down())
			{
				commands.push(mActionBinding[Action::MoveDown]);
			}

			if (mJoystick->Left())
			{
				commands.push(mActionBinding[Action::MoveLeft]);
			}

			if (mJoystick->Right())
			{
				commands.push(mActionBinding[Action::MoveRight]);
			}

			if (mJoystick->RAnalogLeft())
			{
				commands.push(mActionBinding[Action::RotateLeft]);
			}

			if (mJoystick->RAnalogRight())
			{
				commands.push(mActionBinding[Action::RotateRight]);
			}
		}
	}
}

void Player::assignKey(Action action, sf::Keyboard::Key key)
{
	//Remove all keys that are already mapped to an action
	for (auto itr = mKeyBindingPressed.begin(); itr != mKeyBindingPressed.end();)
	{
		if (itr->second == action)
		{
			mKeyBindingPressed.erase(itr++);
		}
		else
		{
			++itr;
		}
		//insert new binding
		mKeyBindingPressed[key] = action;
	}
}

//Mike
void Player::assignReleaseKey(Action action, sf::Keyboard::Key key) //Mike
{
	//Remove all keys that are already mapped to an action
	for (auto itr = mKeyBindingReleased.begin(); itr != mKeyBindingReleased.end();)
	{
		if (itr->second == action)
		{
			mKeyBindingReleased.erase(itr++);
		}
		else
		{
			++itr;
		}
		//insert new binding
		mKeyBindingReleased[key] = action;
	}
}

sf::Keyboard::Key Player::getAssignedKey(Action action) const
{
	for (auto pair : mKeyBindingPressed)
	{
		if (pair.second == action)
		{
			return pair.first;
		}
	}
	return sf::Keyboard::Unknown;
}

void Player::setMissionStatus(MissionStatus status)
{
	mCurrentMissionStatus = status;
}

Player::MissionStatus Player::getMissionStatus() const
{
	return mCurrentMissionStatus;
}

Xbox360Controller * Player::getJoystick() const
{
	if (mJoystick != nullptr)
	{
		return mJoystick;
	}
	return nullptr;
}

void Player::setJoystick(Xbox360Controller * joystick)
{
	mJoystick = joystick;
}

int Player::getLocalIdentifier() const
{
	return mLocalIdentifier;
}

void Player::setLocalIdentifier(int id)
{
	mLocalIdentifier = id;
}

void Player::initializeActions()
{
	mActionBinding[Action::MoveLeft].action = derivedAction<Character>(CharacterMover(-1.f, 0.f, 0.f, mLocalIdentifier));
	mActionBinding[Action::MoveRight].action = derivedAction<Character>(CharacterMover(1.f, 0.f, 0.f, mLocalIdentifier));
	mActionBinding[Action::MoveUp].action = derivedAction<Character>(CharacterMover(0.f, -1.f, 0.f, mLocalIdentifier));
	mActionBinding[Action::MoveDown].action = derivedAction<Character>(CharacterMover(0.f, 1.f, 0.f, mLocalIdentifier));
	mActionBinding[Action::RotateLeft].action = derivedAction<Character>(CharacterMover(0.f, 0.f, -1.f, mLocalIdentifier)); // Alex - Rotate left action
	mActionBinding[Action::RotateRight].action = derivedAction<Character>(CharacterMover(0.f, 0.f, 1.f, mLocalIdentifier)); // Alex - Rotate right action

	mActionBinding[Action::StartGrenade].action = derivedAction<Character>(CharacterGrenadeStarter(mLocalIdentifier));
	mActionBinding[Action::LaunchGrenade].action = derivedAction<Character>(CharacterGrenadeLauncher(mLocalIdentifier));

	mActionBinding[Action::Fire].action = derivedAction<Character>(CharacterFireTrigger(mLocalIdentifier));
}

bool Player::isRealtimeAction(Action action)
{
	// Alex - Add rotation case as real-time action
	switch (action)
	{
	case Action::MoveLeft:
	case Action::MoveRight:
	case Action::MoveUp:
	case Action::MoveDown:
	case Action::RotateLeft:
	case Action::RotateRight:
	case Action::Fire:
		return true;
	default:
		return false;
	}
}