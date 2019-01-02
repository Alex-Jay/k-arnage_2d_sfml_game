#pragma once
#include "Entity.hpp"
#include "Command.hpp"
#include "CommandQueue.hpp"
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

	virtual void remove();
	virtual bool isMarkedForRemoval() const;

	bool isDestructible();
	void checkPickupDrop(CommandQueue& commands);
	void createPickup(SceneNode& node, const TextureHolder& textures) const;
protected:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	void updateCurrent(sf::Time dt, CommandQueue& commands);

private:
	ObstacleID mType;
	sf::Sprite mSprite;
	Command mDropPickupCommand;

	bool mDroppedPickup;
};
