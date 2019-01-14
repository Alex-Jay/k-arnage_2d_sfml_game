#pragma once
#include "Command.hpp"
#include "Xbox360Controller.hpp"

#include "SFML/Window/Event.hpp"

#include <map>

class CommandQueue;

class Player
{
public:

	//enum class JoysticAxisPos { LAnalogUp, LAnalogDown, LAnalogLeft, LAnalogRight, RAnalogUp, RAnalogDown, RAnalogLeft, RAnalogRight, DpadUp, DpadDown, DpadLeft, DpadRight };
	enum class JoystickButton { A, B, X, Y, LB, RB, Back, Start, L3, R3 };


	enum class Action
	{
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		RotateLeft,
		RotateRight,
		Fire,
		StartGrenade,
		ActionCount,
		LaunchGrenade
	};

	enum class MissionStatus { MissionRunning, MissionSuccess, MissionFailure };

public:
	Player(int localIdentifier);
	void handleEvent(const sf::Event& event, CommandQueue& commands);
	void handleRealtimeInput(CommandQueue& commands);
	void assignKey(Action action, sf::Keyboard::Key key);
	void assignReleaseKey(Action action, sf::Keyboard::Key key);
	sf::Keyboard::Key getAssignedKey(Action action) const;

	void setMissionStatus(MissionStatus status);
	MissionStatus getMissionStatus() const;

	Xbox360Controller* getJoystick() const;
	void setJoystick(Xbox360Controller* joystick);

	int getLocalIdentifier() const;
	void setLocalIdentifier(int id);

private:
	void initializeActions();
	static bool isRealtimeAction(Action action);

private:
	int mLocalIdentifier;
	Xbox360Controller* mJoystick;

	std::map<sf::Joystick::Axis, Action> mJoystickBindingMoved;
	std::map<JoystickButton, Action> mJoystickBindingPressed;
	std::map<sf::Keyboard::Key, Action> mKeyBindingPressed;
	std::map<sf::Keyboard::Key, Action> mKeyBindingReleased;
	std::map<Action, Command> mActionBinding;
	MissionStatus mCurrentMissionStatus;
};

