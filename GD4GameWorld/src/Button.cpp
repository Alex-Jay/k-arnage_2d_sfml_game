#include "Button.hpp"
#include "Utility.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>


namespace GUI
{

	Button::Button(State::Context context)
		: mCallback()
		, mSprite(context.textures->get(TextureIDs::Buttons))
		, mText("", context.fonts->get(FontIDs::Main), 16)
		, mIsToggle(false)
		, mSounds(*context.sounds)
	{
		changeTexture(Type::Normal);

		sf::FloatRect bounds = mSprite.getLocalBounds();
		mText.setPosition(bounds.width / 2.f, bounds.height / 2.f);
	}

	void Button::setCallback(Callback callback)
	{
		mCallback = std::move(callback);
	}

	void Button::setText(const std::string& text)
	{
		mText.setString(text);
		centreOrigin(mText);
	}

	void Button::setToggle(bool flag)
	{
		mIsToggle = flag;
	}

	bool Button::isSelectable() const
	{
		return true;
	}

	void Button::select()
	{
		Component::select();

		changeTexture(Type::Selected);
	}

	void Button::deselect()
	{
		Component::deselect();

		changeTexture(Type::Normal);
	}

	void Button::activate()
	{
		Component::activate();

		// If we are toggle then we should show that the button is pressed and thus "toggled".
		if (mIsToggle)
			changeTexture(Type::Pressed);

		if (mCallback)
			mCallback();

		// If we are not a toggle then deactivate the button since we are just momentarily activated.
		if (!mIsToggle)
			deactivate();

		mSounds.play(SoundEffectIDs::Button);
	}

	void Button::deactivate()
	{
		Component::deactivate();

		if (mIsToggle)
		{
			// Reset texture to right one depending on if we are selected or not.
			if (isSelected())
				changeTexture(Type::Selected);
			else
				changeTexture(Type::Normal);
		}
	}

	void Button::handleEvent(const sf::Event&)
	{
	}

	void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		target.draw(mSprite, states);
		target.draw(mText, states);
	}

	void Button::changeTexture(Type buttonType)
	{
		sf::IntRect textureRect(0, 50 * static_cast<int>(buttonType), 200, 50);
		mSprite.setTextureRect(textureRect);
	}

}
