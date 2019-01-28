#include "GameState.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "MusicPlayer.hpp"
#include "Utility.hpp"


GameState::GameState(StateStack& stack, Context context)
	: State(stack, context)
	  , mWorld(*context.window, *context.fonts, *context.sounds)
	  , mPlayerOne(context.playerOne->getLocalIdentifier())
	  , mPlayerTwo(context.playerTwo->getLocalIdentifier())
	  , mScoreText()
	 
{
	mPlayerOne.setMissionStatus(Player::MissionStatus::MissionRunning);
	mPlayerTwo.setMissionStatus(Player::MissionStatus::MissionRunning);

	//Play the mission theme
	context.music->play(MusicIDs::MissionTheme);

	//Not adding scoring in this iteration due to time constraints
	//sf::Font& font = context.fonts->get(FontIDs::Main);
	//sf::Vector2f windowSize(context.window->getSize());

	//mScoreText.setFont(font);
	//mScoreText.setString("Score: 10");
	//mScoreText.setCharacterSize(60);
	//centreOrigin(mScoreText);
	//mScoreText.setPosition(windowSize.x, windowSize.y);
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

	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayerOne.handleRealtimeInput(commands);
	mPlayerTwo.handleRealtimeInput(commands);

	if (!mWorld.hasAlivePlayer())
	{
		getContext().playerOne->setMissionStatus(Player::MissionStatus::MissionFailure);
		getContext().playerOne->setMissionStatus(Player::MissionStatus::MissionFailure);
		requestStackPush(StateIDs::GameOver);
	}

	//else if (mWorld.hasPlayerReachedEnd())
	//{
	//	mPlayerOne.setMissionStatus(Player::MissionStatus::MissionSuccess);
	//	mPlayerTwo.setMissionStatus(Player::MissionStatus::MissionSuccess); 
	//	requestStackPush(StateIDs::GameOver);
	//}

	return true;
}

bool GameState::handleEvent(const sf::Event& event)
{
	//Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayerOne.handleEvent(event, commands);
	mPlayerTwo.handleEvent(event, commands);

	//Escape pressed, trigger pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
	{
		requestStackPush(StateIDs::Pause);
	}
	return true;
}
