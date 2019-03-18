#pragma once
#include "State/State.hpp"
#include "Component/Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class LobbyState : public State
{
public:
	LobbyState(StateStack& stack, Context context);
	virtual void			draw();
	virtual bool			update(sf::Time dt);
	virtual bool			handleEvent(const sf::Event& event);

	int getButtonFromXY(int x, int y);

private:
	void displayText(Context context);

private:
	sf::Sprite				mBackgroundSprite;
	GUI::Container			mGUIContainer;

	sf::Text			mText;
};

