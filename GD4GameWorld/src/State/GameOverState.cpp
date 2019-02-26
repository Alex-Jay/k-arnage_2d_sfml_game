#include "GameOverState.hpp"
#include "Utility.hpp"
#include "Player.hpp"
#include "ResourceHolder.hpp"
//#include "PlayerManager.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>


GameOverState::GameOverState(StateStack& stack, Context context)
	: State(stack, context)
	  , mElapsedTime(sf::Time::Zero)
{
	sf::Font& font = context.fonts->get(FontIDs::Main);
	sf::Vector2f windowSize(context.window->getSize());
	
	mGameOverText.setFont(font);
	
	// If one of the players dies, End game
	if (context.playerOne->getMissionStatus() == Player::MissionStatus::MissionFailure/* ||
		context.playerTwo->getMissionStatus() == Player::MissionStatus::MissionFailure)*/)
	{
		mGameOverText.setString("Game Over.");
	}
	else
	{
		mGameOverText.setString("You have survived the zombie wave.");
	}

	mGameOverText.setCharacterSize(50);
	centreOrigin(mGameOverText);
	mGameOverText.setPosition(0.5f * windowSize.x, 0.4f * windowSize.y);
}

void GameOverState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());

	// Create dark, semitransparent background
	sf::RectangleShape backgroundShape;
	backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
	backgroundShape.setSize(window.getView().getSize());

	window.draw(backgroundShape);
	window.draw(mGameOverText);
}

bool GameOverState::update(sf::Time dt)
{
	// Show state for 3 seconds, after return to menu
	mElapsedTime += dt;
	if (mElapsedTime > sf::seconds(3))
	{
		requestStackClear();
		requestStackPush(StateIDs::ScoreBoard);
	}
	return false;
}

bool GameOverState::handleEvent(const sf::Event&)
{
	return false;
}
