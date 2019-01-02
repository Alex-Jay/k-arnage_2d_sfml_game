#include "GameState.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "MusicPlayer.hpp"
#include "Utility.hpp"


GameState::GameState(StateStack& stack, Context context)
	: State(stack, context)
	  , mWorld(*context.window, *context.fonts, *context.sounds)
	  , mPlayer(*context.player)
{
	mPlayer.setMissionStatus(Player::MissionStatus::MissionRunning);

	//Play the mission theme
	context.music->play(MusicIDs::MissionTheme);

	//Not adding scoring in this iteration due to time constraints
	//sf::Font& font = context.fonts->get(FontIDs::Main);
	//sf::Vector2f windowSize(context.window->getSize());

	//mScoreText.setFont(font);
	//mScoreText.setString("Score: 10");
	//mScoreText.setCharacterSize(20);
	//centreOrigin(mScoreText);
	//mScoreText.setPosition(150.f , 20.f);
}

void GameState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());
	
	mWorld.draw();

	//window.draw(mScoreText);
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

bool GameState::handleEvent(const sf::Event& event)
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
