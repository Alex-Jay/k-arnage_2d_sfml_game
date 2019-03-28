#include "State/SpectateState.hpp"
#include "Component/Button.hpp"
#include "Structural/Utility.hpp"
#include "Audio/MusicPlayer.hpp"
#include "Structural/ResourceHolder.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

SpectateState::SpectateState(StateStack& stack, Context context, bool letUpdatesThrough)
	: State(stack, context)
	//, mBackgroundSprite()
	, mCurrentlySpectating()
	, mGUIContainer()
	, mLetUpdatesThrough(letUpdatesThrough)
{
	sf::Font& font = context.fonts->get(Fonts::Main);
	sf::Vector2f windowSize(context.window->getSize());

	mCurrentlySpectating.setFont(font);
	mCurrentlySpectating.setString("Spectating Player: Joe");
	mCurrentlySpectating.setCharacterSize(50);
	centerOrigin(mCurrentlySpectating);
	mCurrentlySpectating.setPosition(0.5f * windowSize.x, 0.4f * windowSize.y);

	auto returnButton = std::make_shared<GUI::Button>(context);
	returnButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 150);
	returnButton->setText("Next Player");
	returnButton->setCallback([this]()
	{
		// TODO: Set new viewing player here
		//requestStackPop();
	});

	auto backToMenuButton = std::make_shared<GUI::Button>(context);
	backToMenuButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 200);
	backToMenuButton->setText("Back to menu");
	backToMenuButton->setCallback([this]()
	{
		requestStateClear();
		requestStackPush(States::Menu);
	});

	mGUIContainer.pack(returnButton);
	mGUIContainer.pack(backToMenuButton);

	getContext().music->setPaused(false);
}

SpectateState::~SpectateState()
{
	requestStateClear();
}

void SpectateState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());

	//sf::RectangleShape backgroundShape;
	//backgroundShape.setFillColor(sf::Color::Transparent);
	//backgroundShape.setSize(window.getView().getSize());

	//window.draw(backgroundShape);
	window.draw(mCurrentlySpectating);
	window.draw(mGUIContainer);
}

bool SpectateState::update(sf::Time)
{
	return mLetUpdatesThrough;
}

bool SpectateState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);

	return false;
}