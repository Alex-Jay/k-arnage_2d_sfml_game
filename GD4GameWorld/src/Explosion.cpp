#include "Explosion.hpp"
#include "DataTables.hpp"

#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
	const std::vector<ExplosionData> Table = initializeExplosionData();
}

Explosion::Explosion(ExplosionIDs type)
	: Entity(1)
	, mType(type)
{
}

unsigned int Explosion::getCategory() const
{
	return static_cast<int>(Category::Explosion);
}

int Explosion::getRadious() const
{
	return Table[static_cast<int>(mType)].radious;
}

int Explosion::getDamage() const
{
	return Table[static_cast<int>(mType)].damage;
}

sf::FloatRect Explosion::getBoundingRect() const
{
	return getWorldTransform().transformRect(sf::FloatRect(0,0,10,10));
}


void Explosion::updateCurrent(sf::Time dt, CommandQueue & commands)
{
	if (!explosionTimerStarted)
	{
		StartTimer(dt);
	}
	else
	{
		mExplosionTimer += dt;
	}

	if (mExplosionTimer.asMilliseconds() > 500)
	{
		Entity::remove();
	}
}

void Explosion::StartTimer(sf::Time dt)
{
	explosionTimerStarted = true;
	mExplosionTimer = dt;
}