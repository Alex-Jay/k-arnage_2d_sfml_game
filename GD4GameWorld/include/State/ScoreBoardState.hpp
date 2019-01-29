#pragma once
#include "State.hpp"
#include "Player.hpp"
#include "Container.hpp"

#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"

class ScoreBoardState : public State
{
public:
	ScoreBoardState(StateStack& stack, Context context);
	~ScoreBoardState();

	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);

private:
	sf::Sprite mBackgroundSprite;
	sf::Text mScoreboardTitle;
	sf::Text mPlayerOneScore;
	sf::Text mPlayerTwoScore;
	GUI::Container mGUIContainer;
};
