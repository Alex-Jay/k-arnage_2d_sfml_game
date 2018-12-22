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
	, mSprite(textures.get(TextureIDs::Crate))
{
	mSprite.scale(sf::Vector2f(0.25f, 0.25f));
	centreOrigin(mSprite);
}

unsigned int Obstacle::getCategory() const
{
	return static_cast<int>(Category::Obstacle);
}

sf::FloatRect Obstacle::getBoundingRect() const
{
	//TODO Fix Scaleing
	sf::FloatRect f = mSprite.getGlobalBounds();
	f.height *= 0.25f;
	f.width *= 0.25f;
	return getWorldTransform().transformRect(f);
}

void Obstacle::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}
