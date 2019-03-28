#pragma once
#include "Entity/Entity.hpp"
#include "Command/Command.hpp"
#include "Command/CommandQueue.hpp"
#include "Structural/ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>


class Obstacle : public Entity
{
public:
	enum class ObstacleID
	{
		Crate,
		Wreck,
		Rock,
		TypeCount
	};

	struct ObstacleData {
		ObstacleData(int16_t type, int16_t x, int16_t y, int16_t a)
			: type(type)
			, x(x)
			, y(y)
			, a(a) {}
		int16_t type, x, y, a;
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
