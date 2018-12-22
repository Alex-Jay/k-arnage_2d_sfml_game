#pragma once
#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>


class Obstacle : public Entity
{
public:
	enum class ObstacleID
	{
		Crate,
		Rock
	};

public:
	Obstacle(ObstacleID type, const TextureHolder& textures);
	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect() const;

protected:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	ObstacleID mType;
	sf::Sprite mSprite;
};