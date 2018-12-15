#pragma once

#include "World.hpp"
#include "Player.hpp"
#include "State.hpp"

#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Sprite.hpp"


class GameState : public State
{
public:
	GameState(StateStack& stack, Context context);

private:
	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);

private:
	World mWorld;
	Player& mPlayer;
};