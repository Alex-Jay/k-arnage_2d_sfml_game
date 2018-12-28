#include "Projectile.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "EmitterNode.hpp"
#include "Explosion.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderStates.hpp"

#include <cassert>
#include <iostream>

namespace
{
	const std::vector<ProjectileData> Table = initializeProjectileData();
}

Projectile::Projectile(ProjectileIDs type, const TextureHolder& textures)
	: Entity(1)
	  , mType(type)
	  , mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect)
{
	centreOrigin(mSprite);

	mExplosionCommand.category = static_cast<int>(Category::SceneAirLayer);
	mExplosionCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createExplosion(node, textures);
	};

	mSprite.setScale(Table[static_cast<int>(type)].textureScale, Table[static_cast<int>(type)].textureScale);
}

bool Projectile::isGrenade() const
{
	return mType == ProjectileIDs::Grenade;
}

unsigned int Projectile::getCategory() const
{
	if (mType == ProjectileIDs::EnemyBullet)
		return static_cast<int>(Category::EnemyProjectile);
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

float Projectile::getMaxSpeed(float initialSpeed) const
{
	return Table[static_cast<int>(mType)].speed + initialSpeed;
}

int Projectile::getDamage() const
{
	return Table[static_cast<int>(mType)].damage;
}

void Projectile::setInitialVelocity(float vel)
{
	if (isGrenade())
	{
		mInitialVelocity = vel;
	}
	else
	{
		mInitialVelocity = 0;
	}
}

bool Projectile::isMarkedForRemoval() const
{
	return isDestroyed() && (mAnimation.isFinished() || !mShowExplosion);
}

void Projectile::remove()
{
	Entity::remove();
	mShowExplosion = false;
}

//Mike
void Projectile::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (isGrenade())
	{
		handleGrenade(dt, commands);
	}
	Entity::updateCurrent(dt, commands);
}

void Projectile::handleGrenade(sf::Time dt, CommandQueue& commands)
{
	if (!mGrenadeTimerStarted)
	{
		StartTimer(dt);
	}
	else
	{
		mGrenadeTimer += dt;
	}

	setVelocity(MoveTowards(getVelocity(), sf::Vector2f(0.f, 0.f), 10.f));

	if (mGrenadeTimer.asSeconds() >= 3)
	{
		mGrenadeTimerStarted = false;
		damage(1);
	}

	if (isDestroyed())
	{
		commands.push(mExplosionCommand);
	}
}

void Projectile::createExplosion(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Explosion::ExplosionIDs>(static_cast<int>(Explosion::ExplosionIDs::GrenadeExplosion));

	std::unique_ptr<Explosion> explosion(new Explosion(type, textures));
	explosion->setPosition(getWorldPosition());
	node.attachChild(std::move(explosion));
}

void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Alex - Debug grenade bounding rect
	if (Projectile::isGrenade())
	{
		drawBoundingRect(target, states);
	}

	target.draw(mSprite, states);
}

void Projectile::StartTimer(sf::Time dt)
{
	mGrenadeTimerStarted = true;
	mGrenadeTimer = dt;
}
