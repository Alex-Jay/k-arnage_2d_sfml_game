#include "KeyBinding.hpp"

KeyBinding::KeyBinding(int controlPreconfiguration)
	: mKeyMap()
{
	// Player 1 initial bindings
	if (controlPreconfiguration == 1)
	{
		mKeyMap[sf::Keyboard::A] = Action::MoveLeft;
		mKeyMap[sf::Keyboard::D] = Action::MoveRight;
		mKeyMap[sf::Keyboard::W] = Action::MoveUp;
		mKeyMap[sf::Keyboard::S] = Action::MoveDown;
		mKeyMap[sf::Keyboard::Q] = Action::RotateLeft;
		mKeyMap[sf::Keyboard::E] = Action::RotateRight;
		mKeyMap[sf::Keyboard::Space] = Action::Fire;
		mKeyMap[sf::Keyboard::M] = Action::StartGrenade;
		mKeyMap[sf::Keyboard::M] = Action::LaunchGrenade;
	}
	// Player 2 intial bindings
	else if (controlPreconfiguration == 1)
	{
		mKeyMap[sf::Keyboard::Left] = Action::MoveLeft;
		mKeyMap[sf::Keyboard::Right] = Action::MoveRight;
		mKeyMap[sf::Keyboard::Up] = Action::MoveUp;
		mKeyMap[sf::Keyboard::Down] = Action::MoveDown;
		mKeyMap[sf::Keyboard::Numpad4] = Action::RotateLeft;
		mKeyMap[sf::Keyboard::Numpad6] = Action::RotateRight;
		mKeyMap[sf::Keyboard::Insert] = Action::Fire;
		mKeyMap[sf::Keyboard::Enter] = Action::StartGrenade;
		mKeyMap[sf::Keyboard::Enter] = Action::LaunchGrenade;
	}
}

void KeyBinding::assignKey(Action action, sf::Keyboard::Key key)
{
	// Remove all keys that already map an action
	for (auto itr = mKeyMap.begin(); itr != mKeyMap.end(); )
	{
		if (itr->second == action)
			mKeyMap.erase(itr++);
		else
			++itr;
	}

	// Store new binding
	mKeyMap[key] = action;
}

sf::Keyboard::Key KeyBinding::getAssignedKey(Action action) const
{
	for (auto pair : mKeyMap)
	{
		if (pair.second == action)
		{
			return pair.first;
		}
	}
	return sf::Keyboard::Unknown;
}

bool KeyBinding::checkAction(sf::Keyboard::Key key, Action & out) const
{
	auto found = mKeyMap.find(key);
	if (found == mKeyMap.end())
	{
		return false;
	}
	else
	{
		out = found->second;
		return true;
	}
}

std::vector<KeyBinding::Action> KeyBinding::getRealtimeActions() const
{
	// Return all reatime actions that are currently active
	std::vector<Action> actions;

	for (auto pair : mKeyMap)
	{
		// If key is pressed and is realtime action, store it
		if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
		{
			actions.push_back(pair.second);
		}
	}

	return actions;
}

void KeyBinding::initializeActions()
{
}

bool isRealtimeAction(PlayerAction::Type action)
{
	switch (action)
	{
	case PlayerAction::MoveLeft:
	case PlayerAction::MoveRight:
	case PlayerAction::MoveUp:
	case PlayerAction::MoveDown:
	case PlayerAction::RotateLeft:
	case PlayerAction::RotateRight:
	case PlayerAction::Fire:
		return true;

	default:
		return false;
	}
}
