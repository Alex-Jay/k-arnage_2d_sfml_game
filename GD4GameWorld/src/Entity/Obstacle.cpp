#include "Obstacle.hpp"
#include "DataTables.hpp"
#include "Category.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>


namespace
{
	const std::vector<ObstacleData> Table = initializeObstacleData();
}

Obstacle::Obstacle(ObstacleID type, const TextureHolder& textures)
	: Entity(1)
	, mType(type)
	, mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect)
{
	centreOrigin(mSprite);
}

unsigned int Obstacle::getCategory() const
{
	return static_cast<int>(Category::Pickup);
}

sf::FloatRect Obstacle::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Obstacle::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}
