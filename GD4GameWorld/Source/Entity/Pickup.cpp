#include "Entity/Pickup.hpp"
#include "DataTable/DataTables.hpp"
#include "Structural/Category.hpp"
#include "Command/CommandQueue.hpp"
#include "Structural/Utility.hpp"
#include "Structural/ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>


namespace
{
	const std::vector<PickupData> Table = initializePickupData();
}

Pickup::Pickup(Type type, const TextureHolder& textures)
	: Entity(1)
	, mType(type)
	, mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect)
{
	centreOrigin(mSprite);
}

unsigned int Pickup::getCategory() const
{
	return static_cast<int>(Category::Pickup);
}

sf::FloatRect Pickup::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Pickup::apply(Character& player) const
{
	//Table[static_cast<int>(mType)].action(player); //TOFIX
}

void Pickup::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}
