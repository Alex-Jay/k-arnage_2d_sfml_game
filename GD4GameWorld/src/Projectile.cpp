#include "Projectile.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "Constants.hpp"
#include "EmitterNode.hpp"
#include "Explosion.hpp"
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


	// Add Animation system for grenades
	if (isGrenade())
	{
		mAnimation.setTexture(textures.get(TextureIDs::Explosion));
		mAnimation.setFrameSize(sf::Vector2i(256, 256));
		mAnimation.setNumFrames(16);
		mAnimation.setDuration(sf::seconds(1));
		centreOrigin(mAnimation);
		mShowExplosion = true;
		//OLD PARTICLE SYSTEM CODE
		//std::unique_ptr<EmitterNode> smoke(new EmitterNode(Particle::Type::Smoke));
		//smoke->setPosition(0.f, getBoundingRect().height / 2.f);
		//attachChild(std::move(smoke));

		//std::unique_ptr<EmitterNode> propellant(new EmitterNode(Particle::Type::Exhaust));
		//propellant->setPosition(0.f, getBoundingRect().height / 2.f);
		//attachChild(std::move(propellant));

	}


	mExplosionCommand.category = static_cast<int>(Category::SceneAirLayer);
	mExplosionCommand.action = [this](SceneNode& node, sf::Time)
	{
		createExplosion(node);
	};

	mDropPickupCommand.category = static_cast<int>(Category::SceneAirLayer);
	mDropPickupCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};

	mSprite.setScale(Table[static_cast<int>(type)].textureScale, Table[static_cast<int>(type)].textureScale);
}

void Projectile::createPickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Pickup::PickupID>(randomInt(static_cast<int>(Pickup::PickupID::TypeCount)));

	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(getWorldPosition());
	pickup->setVelocity(0.f, 1.f);
	node.attachChild(std::move(pickup));
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
		if (!mGrenadeTimerStarted)
		{
			StartTimer(dt);
		}
		else
		{
			mGrenadeTimer += dt;
		}

		setVelocity(MoveTowards(getVelocity(), sf::Vector2f(0.f, 0.f), 10.f));

		if (mGrenadeTimer.asSeconds() > 3)
		{
			mGrenadeTimerStarted = false;
			mAnimation.update(dt);
			damage(1);
		}

		if (isDestroyed())
		{
			mAnimation.update(dt);

			commands.push(mExplosionCommand);


			
			// Play explosion sound only once
			if (!mPlayedExplosionSound)
			{
				SoundEffectIDs soundEffect = (randomInt(2) == 0) ? SoundEffectIDs::Explosion1 : SoundEffectIDs::Explosion2;
				playLocalSound(commands, soundEffect);

				mPlayedExplosionSound = true;
			}
			return;
		}

	}
	Entity::updateCurrent(dt, commands);
}


void Projectile::createExplosion(SceneNode& node) const
{
	auto type = static_cast<Explosion::ExplosionIDs>(static_cast<int>(Explosion::ExplosionIDs::GrenadeExplosion));

	std::unique_ptr<Explosion> explosion(new Explosion(type));
	explosion->setPosition(getWorldPosition());
	node.attachChild(std::move(explosion));
}

void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (isDestroyed() && mShowExplosion)
		target.draw(mAnimation, states);
	else
		target.draw(mSprite, states);
}

void Projectile::StartTimer(sf::Time dt)
{
	mGrenadeTimerStarted = true;
	mGrenadeTimer = dt;
}

void Projectile::playLocalSound(CommandQueue& commands, SoundEffectIDs effect)
{
	sf::Vector2f worldPosition = getWorldPosition();

	Command command;
	command.category = static_cast<int>(Category::SoundEffect);
	command.action = derivedAction<SoundNode>([effect, worldPosition](SoundNode& node, sf::Time)
	{
		node.playSound(effect, worldPosition);
	});
	commands.push(command);

}