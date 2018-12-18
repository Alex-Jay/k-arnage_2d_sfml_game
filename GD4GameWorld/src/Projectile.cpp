#include "Projectile.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Constants.hpp"
#include "EmitterNode.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderStates.hpp"

#include <cassert>
#include <cmath>

namespace
{
	const std::vector<ProjectileData> Table = initializeProjectileData();
}

Projectile::Projectile(ProjectileIDs type, const TextureHolder& textures)
	:Entity(1)
	, mType(type)
	, mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect)
	, mTargetDirection()
{
	centreOrigin(mSprite);
	// Add particle system for grenades
	if (isGrenade())
	{
		//std::unique_ptr<EmitterNode> smoke(new EmitterNode(Particle::Type::Smoke));
		//smoke->setPosition(0.f, getBoundingRect().height / 2.f);
		//attachChild(std::move(smoke));

		//std::unique_ptr<EmitterNode> propellant(new EmitterNode(Particle::Type::Exhaust));
		//propellant->setPosition(0.f, getBoundingRect().height / 2.f);
		//attachChild(std::move(propellant));

	}

	mSprite.setScale(Table[static_cast<int>(type)].textureScale, Table[static_cast<int>(type)].textureScale);
}

void Projectile::guideTowards(sf::Vector2f position)
{
	assert(isGrenade());
	mTargetDirection = unitVector(position - getWorldPosition());
}

bool Projectile::isGrenade() const
{
	return mType == ProjectileIDs::Grenade;
}

unsigned int Projectile::getCategory() const
{
	if (mType == ProjectileIDs::EnemyBullet)
		return static_cast<int>(Category::EnemyProjectile);
	else
		return static_cast<int>(Category::AlliedProjectile);
}

sf::FloatRect Projectile::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

float Projectile::getMaxSpeed() const
{
	return Table[static_cast<int>(mType)].speed;
}

int Projectile::getDamage() const
{
	return Table[static_cast<int>(mType)].damage;
}


void Projectile::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (isGrenade())
	{
		//const float approachRate = APPROACHRATE;
		//sf::Vector2f newVelocity = unitVector(approachRate * dt.asSeconds() * mTargetDirection + getVelocity());
		//newVelocity *= getMaxSpeed();
		//float angle = std::atan2(newVelocity.y, newVelocity.x);
		//setRotation(toDegrees(angle) + 90.f);

		setVelocity(MoveTowards(getVelocity(), sf::Vector2f(0.f, 0.f), 10.f));

		if (!GrenadeTimerStarted)
		{
			StartTimer(dt);
		}
		else
		{
			GrenadeTimer += dt;
		}

		//sf::Time currentTime = sf::Time::Zero;
		//currentTime += dt;

		if (GrenadeTimer.asSeconds() > 1)
		{
			GrenadeTimerStarted = false;
			//Genade.explode()
			remove();
		}

	}
	Entity::updateCurrent(dt, commands);
}

void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}

void Projectile::StartTimer(sf::Time dt)
{
	GrenadeTimerStarted = true;
	GrenadeTimer = dt;
}