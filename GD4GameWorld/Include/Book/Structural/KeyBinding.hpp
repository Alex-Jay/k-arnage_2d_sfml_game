#ifndef BOOK_KEYBINDING_HPP
#define BOOK_KEYBINDING_HPP

#include <SFML/Window/Keyboard.hpp>
#include "Structural/Xbox360Controller.hpp"
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Event.hpp>

#include <map>
#include <vector>


namespace PlayerAction
{
	enum Type
	{
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		Fire,
		StartGrenade,
		Count,
		LaunchGrenade,
	};
}

namespace Joystick
{
	enum class Button
	{
		A,
		B,
		X,
		Y,
		LB,
		RB,
		Back,
		Start,
		L3,
		R3
	};
}

class KeyBinding
{
	public:
		typedef PlayerAction::Type Action;


	public:
		explicit				KeyBinding(int controlPreconfiguration);

		void					assignKey(Action action, sf::Keyboard::Key key);
		void					assignReleaseKey(Action action, sf::Keyboard::Key key);
		sf::Keyboard::Key		getAssignedKey(Action action) const;

		bool					checkAction(sf::Keyboard::Key key, Action& out) const;
		bool					checkReleaseAction(sf::Keyboard::Key key, Action & out) const;
		std::vector<Action>		getRealtimeActions() const;


	private:
		void					initializeActions();


	private:
		Xbox360Controller*						mJoystick;
		std::map<Joystick::Button, Action>		mJoystickBindingPressed;
		std::map<Joystick::Button, Action>		mJoystickBindingReleased;
		std::map<sf::Keyboard::Key, Action>		mKeyMap;
		std::map<sf::Keyboard::Key, Action>		mKeyBindingReleased;
};

bool					isRealtimeAction(PlayerAction::Type action);

#endif // BOOK_KEYBINDING_HPP
