#include "Obstacle.hpp"
#include "DataTables.hpp"
#include "Category.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "Pickup.hpp"
#include <SFML/Graphics/RectangleShape.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

//Mike - Obstacle Class
namespace
{
	const std::vector<ObstacleData> Table = initializeObstacleData();
}

Obstacle::Obstacle(ObstacleID type, const TextureHolder& textures)
	: Entity(1)
	  , mType(type)
	  , mSprite(textures.get(TextureIDs::Crate))
	  , mDroppedPickup(false)
{
	centreOrigin(mSprite);

	mDropPickupCommand.category = static_cast<int>(Category::SceneLayer);
	mDropPickupCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};
}

unsigned int Obstacle::getCategory() const
{
	return static_cast<int>(Category::Obstacle);
}

bool Obstacle::isDestructible()
{
	switch (mType)
	{
	case ObstacleID::Crate:
		return true;
	default:
		return false;
	}
}

void Obstacle::checkPickupDrop(CommandQueue& commands)
{
	//if (randomInt(3) == 0)
	commands.push(mDropPickupCommand);
}

void Obstacle::createPickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Pickup::PickupID>(randomInt(static_cast<int>(Pickup::PickupID::TypeCount)));

	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(getWorldPosition());
	pickup->setVelocity(0.f, 1.f);
	node.attachChild(std::move(pickup));
}

void Obstacle::remove()
{
	Entity::remove();
}

sf::FloatRect Obstacle::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Obstacle::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}


void Obstacle::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (isDestroyed())
	{
		checkPickupDrop(commands);
		mDroppedPickup = true;
	}
}

bool Obstacle::isMarkedForRemoval() const
{
	return (isDestroyed() && mDroppedPickup);
}
