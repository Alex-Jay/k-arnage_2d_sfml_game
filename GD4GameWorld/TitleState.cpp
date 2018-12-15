#include "TitleState.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

#include "SFML/Graphics/RenderWindow.hpp"

TitleState::TitleState(StateStack & stack, Context context)
	: State(stack, context)
	, mText()
	, mShowText(true)
	, mTextEffectTime(sf::Time::Zero)
{
	mBackgroundSprite.setTexture(context.textures->get(TextureIDs::TitleScreen));
	mText.setFont(context.fonts->get(FontIDs::Main));
	mText.setString("Press any key to continue");
	centreOrigin(mText);
	mText.setPosition(context.window->getView().getSize() / 2.f);
	mText.setCharacterSize(30);
}

void TitleState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.draw(mBackgroundSprite);

	if (mShowText)
	{
		window.draw(mText);
	}
}

bool TitleState::update(sf::Time dt)
{
	mTextEffectTime += dt;

	if (mTextEffectTime > sf::seconds(0.5f))
	{
		mShowText = !mShowText;
		mTextEffectTime = sf::Time::Zero;
	}
	return true;
}

bool TitleState::handleEvent(const sf::Event& event)
{
	//If any key is pressed move to menu state
	if (event.type == sf::Event::KeyPressed)
	{
		requestStackPop();
		requestStackPush(StateIDs::Menu);
	}
	return true;
}
