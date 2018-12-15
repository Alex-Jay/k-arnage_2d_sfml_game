#include "GameState.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "MusicPlayer.hpp"


GameState::GameState(StateStack& stack, Context context) 
	:State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds)
	, mPlayer(*context.player)	
{
	mPlayer.setMissionStatus(Player::MissionStatus::MissionRunning);

	//Play the mission theme
	context.music->play(MusicIDs::MissionTheme);
}

void GameState::draw()
{
	mWorld.draw();
}

bool GameState::update(sf::Time dt)
{
	mWorld.update(dt);

	if (!mWorld.hasAlivePlayer())
	{
		mPlayer.setMissionStatus(Player::MissionStatus::MissionFailure);
		requestStackPush(StateIDs::GameOver);
	}
	else if (mWorld.hasPlayerReachedEnd())
	{
		mPlayer.setMissionStatus(Player::MissionStatus::MissionSuccess);
		requestStackPush(StateIDs::GameOver);
	}

	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleRealtimeInput(commands);

	return true;
}

bool GameState::handleEvent(const sf::Event & event)
{
	//Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleEvent(event, commands);

	//Escape pressed, trigger pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
	{
		requestStackPush(StateIDs::Pause);
	}
	return true;
}


