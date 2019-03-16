#include "ScoreBoardState.hpp"
#include <Book/Button.hpp>
#include <Book/Utility.hpp>
#include "Structural/ResourceHolder.hpp"
#include <Book/MusicPlayer.hpp>
#include "Constant/Constants.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include <iostream>


ScoreBoardState::ScoreBoardState(StateStack& stack, Context context)
	: State(stack, context)
{
	sf::Font& font = context.fonts->get(Fonts::ID::Main);
	sf::Vector2f windowSize(context.window->getSize());

	// Scoreboard Title Text
	mScoreboardTitle.setFont(font);
	mScoreboardTitle.setString("Score Board");
	mScoreboardTitle.setCharacterSize(70);
	//centreOrigin(mScoreboardTitle);
	mScoreboardTitle.setPosition(0.5f * windowSize.x, 0.1f * windowSize.y);

	// Scoreboard Player One Score
	mPlayerOneScore.setFont(font);
//	mPlayerOneScore.setString("Player One:   " + std::to_string(getContext().playerOne->getScore()) + " pts   |   " + std::to_string((getContext().playerOne->getScore()) / ZOMBIE_KILL_MULTIPLIER) + " kill(s)");
	mPlayerOneScore.setCharacterSize(30);
	//centreOrigin(mPlayerOneScore);
	mPlayerOneScore.setPosition(0.5f * windowSize.x, 0.3f * windowSize.y);

	// Scoreboard Player One Score
	mPlayerTwoScore.setFont(font);
//	mPlayerTwoScore.setString("Player Two:   " + std::to_string(getContext().playerTwo->getScore()) + " pts   |   " + std::to_string((getContext().playerTwo->getScore()) / ZOMBIE_KILL_MULTIPLIER) + " kill(s)");
	mPlayerTwoScore.setCharacterSize(30);
	//centreOrigin(mPlayerTwoScore);
	mPlayerTwoScore.setPosition(0.5f * windowSize.x, 0.4f * windowSize.y);

	auto backToMenuButton = std::make_shared<GUI::Button>(context);
	backToMenuButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 150);
	backToMenuButton->setText("Back to menu");
	backToMenuButton->setCallback([this]()
	{
		//requestStackClear();
		requestStackPush(States::ID::Menu);
	});

	mGUIContainer.pack(backToMenuButton);

	//Pause the music
	context.music->setPaused(true);
}

void ScoreBoardState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());

	sf::RectangleShape backgroundShape;
	backgroundShape.setFillColor(sf::Color(0, 0, 0, 215));
	backgroundShape.setSize(window.getView().getSize());

	window.draw(backgroundShape);
	window.draw(mScoreboardTitle);
	window.draw(mPlayerOneScore);
	window.draw(mPlayerTwoScore);
	window.draw(mGUIContainer);
}

bool ScoreBoardState::update(sf::Time)
{
	return false;
}

bool ScoreBoardState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}

ScoreBoardState::~ScoreBoardState()
{
	getContext().music->setPaused(false);
}
