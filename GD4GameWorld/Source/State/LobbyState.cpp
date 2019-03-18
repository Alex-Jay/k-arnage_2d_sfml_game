#include "State/LobbyState.hpp"
#include "Component/Button.hpp"
#include "Structural/Utility.hpp"
#include "Audio/MusicPlayer.hpp"
#include "Structural/ResourceHolder.hpp"




LobbyState::LobbyState(StateStack& stack, Context context)
	: State(stack, context)
	, mGUIContainer()
{
	sf::Texture& texture = context.textures->get(Textures::LobbyScreen);
	mBackgroundSprite.setTexture(texture);

	displayText(context);

	auto readyButton = std::make_shared<GUI::Button>(context);
	readyButton->setPosition(300, 600);
	readyButton->setText("Ready");
	readyButton->setCallback([this]()
	{
		requestStackPop();
		requestStackPush(States::Game);
	});

	auto exitButton = std::make_shared<GUI::Button>(context);
	exitButton->setPosition(300, 500);
	exitButton->setText("Back");
	exitButton->setCallback([this]()
	{
		requestStackPop();
	});

	mGUIContainer.pack(readyButton);
	mGUIContainer.pack(exitButton);

	// Play menu theme
	context.music->play(Music::MenuTheme);
}


void LobbyState::displayText(Context context)
{
	mText.setFont(context.fonts->get(Fonts::Main));
	mText.setString("Multiplayer Lobby");
	centerOrigin(mText);
	mText.setPosition(sf::Vector2f(context.window->getSize().x / 2u, 100));
}

void LobbyState::draw()
{
	sf::RenderWindow& window = *getContext().window;

	window.setView(window.getDefaultView());

	window.draw(mBackgroundSprite);
	window.draw(mGUIContainer);
	window.draw(mText);
}

bool LobbyState::update(sf::Time)
{
	return true;
}

bool LobbyState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}
