#pragma once
#include "State.hpp"
#include "Player.hpp"
//#include "PlayerManager.hpp"
#include "Container.hpp"
#include "Button.hpp"
#include "Label.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>


class SettingsState : public State
{
public:
	SettingsState(StateStack& stack, Context context);

	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);

	bool isReservedKey(sf::Event event);


private:
	void updateLabels();
	void addButtonLabel(Player::Action action, /*float x,*/ float y, const std::string& text, Context context);


private:
	sf::Sprite mBackgroundSprite;
	GUI::Container mGUIContainer;
	std::array<GUI::Button::Ptr, 2*static_cast<int>(Player::Action::ActionCount)> mBindingButtons;
	std::array<GUI::Label::Ptr, 2*static_cast<int>(Player::Action::ActionCount)> mBindingLabels;
};
