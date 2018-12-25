#include "Obstacle.hpp"
#include "DataTables.hpp"
#include "Category.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "Pickup.hpp"
#include <SFML/Graphics/RectangleShape.hpp>

#include <SFML/Graphics/RenderTarget.hpp>


namespace
{
	const std::vector<ObstacleData> Table = initializeObstacleData();
}

Obstacle::Obstacle(ObstacleID type, const TextureHolder& textures, int obstacleHitpoints)
	: Entity(obstacleHitpoints)
	, mType(type)
	, mSprite(textures.get(TextureIDs::Crate))
{
	//mSprite.scale(sf::Vector2f(0.25f, 0.25f));
	centreOrigin(mSprite);

	mDropPickupCommand.category = static_cast<int>(Category::SceneAirLayer);
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
	//TODO Fix Scaleing
	//sf::FloatRect f = mSprite.getGlobalBounds();
	//f.height *= 0.25f;
	//f.width *= 0.25f;
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Obstacle::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	//sf::FloatRect rect = getBoundingRect();

	//sf::RectangleShape shape;
	//shape.setPosition(sf::Vector2f(rect.left, rect.top));
	//shape.setSize(sf::Vector2f(rect.width, rect.height));
	//shape.setFillColor(sf::Color::Transparent);
	//shape.setOutlineColor(sf::Color::Green);
	//shape.setOutlineThickness(1.f);

	//target.draw(shape);

	target.draw(mSprite, states);
}


