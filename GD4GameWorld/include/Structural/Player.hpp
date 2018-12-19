#pragma once
#include "Command.hpp"
#include "SFML/Window/Event.hpp"
#include <map>

class CommandQueue;

class Player
{
public:

	enum class Action{MoveLeft, MoveRight, MoveUp, MoveDown, RotateLeft, RotateRight, Fire, LaunchGrenade, ActionCount, StartGrenade};
	enum class MissionStatus{MissionRunning, MissionSuccess, MissionFailure};

public:
	Player();
	void handleEvent(const sf::Event& event, CommandQueue& commands);
	void handleRealtimeInput(CommandQueue& commands);
	void assignKey(Action action, sf::Keyboard::Key key);
	sf::Keyboard::Key getAssignedKey(Action action) const;

	void setMissionStatus(MissionStatus status);
	MissionStatus getMissionStatus() const;

private:
	void initializeActions();
	static bool isReleaseAction(Action action);
	static bool isRealtimeAction(Action action);

private:
	std::map<sf::Keyboard::Key, Action> mKeyBindingPressed;
	std::map<sf::Keyboard::Key, Action> mKeyBindingReleased;
	std::map<Action, Command> mActionBinding;
	MissionStatus mCurrentMissionStatus;

};