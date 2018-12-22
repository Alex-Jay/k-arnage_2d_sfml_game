#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Character.hpp"

#include <map>

using namespace std::placeholders;

struct CharacterMover
{
	sf::Vector2f velocity;
	float angle;

	CharacterMover(float vx, float vy, float da) : velocity(vx, vy), angle(da)
	{
	}

	void operator()(Character& Character, sf::Time) const
	{
		Character.accelerate(velocity * Character.getMaxSpeed());
		Character.applyRotation(angle * Character.getMaxRotationSpeed());
		//std::cout << "Velocity [X,Y]: " << velocity.x << ", " << velocity.y << std::endl;
		//std::cout << "Angle: " << angle << std::endl;
	}
};

Player::Player()
	: mCurrentMissionStatus(MissionStatus::MissionRunning)
{
	//Set initial key bindings

	mKeyBindingPressed[sf::Keyboard::Left] = Action::MoveLeft;
	mKeyBindingPressed[sf::Keyboard::Right] = Action::MoveRight;
	mKeyBindingPressed[sf::Keyboard::Up] = Action::MoveUp;
	mKeyBindingPressed[sf::Keyboard::Down] = Action::MoveDown;
	mKeyBindingPressed[sf::Keyboard::Space] = Action::Fire;
	mKeyBindingPressed[sf::Keyboard::M] = Action::StartGrenade;
	// Alex - Init. rotation keys -------------------------
	mKeyBindingPressed[sf::Keyboard::Numpad4] = Action::RotateLeft;
	mKeyBindingPressed[sf::Keyboard::Numpad6] = Action::RotateRight;

	mKeyBindingReleased[sf::Keyboard::M] = Action::LaunchGrenade;


	//set initial action bindings
	initializeActions();
	//Assign all categories to the player's Character
	for (auto& pair : mActionBinding)
	{
		pair.second.category = static_cast<unsigned int>(Category::PlayerCharacter);
	}
}

void Player::handleEvent(const sf::Event& event, CommandQueue& commands)
{
	//check if key pressed is in the key bindings, if so trigger command

	if (event.type == sf::Event::KeyPressed)
	{
		auto found = mKeyBindingPressed.find(event.key.code);

		if (found != mKeyBindingPressed.end() && !isRealtimeAction(found->second))
		{
			commands.push(mActionBinding[found->second]);
		}
	}

	if (event.type == sf::Event::KeyReleased)
	{
		auto found = mKeyBindingReleased.find(event.key.code);

		if (found != mKeyBindingReleased.end() && !isRealtimeAction(found->second))
		{
			commands.push(mActionBinding[found->second]);
		}
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	for (auto pair : mKeyBindingPressed)
	{
		if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
		{
			commands.push(mActionBinding[pair.second]);
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

void Player::initializeActions()
{
	mActionBinding[Action::MoveLeft].action = derivedAction<Character>(CharacterMover(-1.f, 0.f, 0.f));
	mActionBinding[Action::MoveRight].action = derivedAction<Character>(CharacterMover(1.f, 0.f, 0.f));
	mActionBinding[Action::MoveUp].action = derivedAction<Character>(CharacterMover(0.f, -1.f, 0.f));
	mActionBinding[Action::MoveDown].action = derivedAction<Character>(CharacterMover(0.f, 1.f, 0.f));
	mActionBinding[Action::RotateLeft].action = derivedAction<Character>(CharacterMover(0.f, 0.f, -1.f));
	// Alex - Rotate left action
	mActionBinding[Action::RotateRight].action = derivedAction<Character>(CharacterMover(0.f, 0.f, 1.f));
	// Alex - Rotate right action
	mActionBinding[Action::Fire].action = derivedAction<Character>([](Character& a, sf::Time) { a.fire(); });
	mActionBinding[Action::StartGrenade].action = derivedAction<Character>([](Character& a, sf::Time)
	{
		a.startGrenade();
	});
	mActionBinding[Action::LaunchGrenade].action = derivedAction<Character>([](Character& a, sf::Time)
	{
		a.launchGrenade();
	});
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
