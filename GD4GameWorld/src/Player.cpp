#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Character.hpp"

#include <map>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std::placeholders;

struct CharacterMover
{
	sf::Vector2f velocity;
	float angle;

	CharacterMover(float vx, float vy, float da) : velocity(vx, vy), angle(da)
	{

	}

	void operator() (Character& Character, sf::Time) const
	{
		Character.accelerate(velocity * Character.getMaxSpeed());
		Character.applyRotation(angle * Character.getMaxRotationSpeed());
		std::cout << "Velocity [X,Y]: " << velocity.x << ", " << velocity.y << std::endl;
		std::cout << "Angle: " << angle << std::endl;
	}
};

Player::Player()
	: mCurrentMissionStatus(MissionStatus::MissionRunning)
{
	//Set initial key bindings
	mKeyBinding[sf::Keyboard::Left] = Action::MoveLeft;
	mKeyBinding[sf::Keyboard::Right] = Action::MoveRight;
	mKeyBinding[sf::Keyboard::Up] = Action::MoveUp;
	mKeyBinding[sf::Keyboard::Down] = Action::MoveDown;
	// Alex - Init. rotation keys -------------------------
	mKeyBinding[sf::Keyboard::Num4] = Action::RotateLeft;
	mKeyBinding[sf::Keyboard::Num6] = Action::RotateRight;
	//-----------------------------------------------------
	mKeyBinding[sf::Keyboard::Space] = Action::Fire;
	mKeyBinding[sf::Keyboard::M] = Action::LaunchGrenade;

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
	if (event.type == sf::Event::KeyPressed)
	{
		//check if key pressed is in the key bindings, if so trigger command
		auto found = mKeyBinding.find(event.key.code);
		if (found != mKeyBinding.end() && !isRealtimeAction(found->second))
		{
			commands.push(mActionBinding[found->second]);
		}
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	//Check if any key binding keys are pressed
	for (auto pair : mKeyBinding)
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
	for (auto itr = mKeyBinding.begin(); itr != mKeyBinding.end();)
	{
		if (itr->second == action)
		{
			mKeyBinding.erase(itr++);
		}
		else
		{
			++itr;
		}
		//insert new binding
		mKeyBinding[key] = action;
	}
}

sf::Keyboard::Key Player::getAssignedKey(Action action) const
{
	for (auto pair : mKeyBinding)
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
	mActionBinding[Action::RotateLeft].action = derivedAction<Character>(CharacterMover(0.f, 0.f, -1.f)); // Alex - Rotate left action
	mActionBinding[Action::RotateRight].action = derivedAction<Character>(CharacterMover(0.f, 0.f, 1.f)); // Alex - Rotate right action
	mActionBinding[Action::Fire].action = derivedAction<Character>([](Character& a, sf::Time) { a.fire(); });
	mActionBinding[Action::LaunchGrenade].action = derivedAction<Character>([](Character& a, sf::Time) { a.launchGrenade(); });
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