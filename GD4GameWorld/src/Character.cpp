#include "Character.hpp"
#include "DataTables.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "CommandQueue.hpp"
#include "ResourceHolder.hpp"
#include "SoundNode.hpp"

#include "Constants.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>
#include <iostream>


namespace
{
	const std::vector<CharacterData> Table = initializeCharacterData();
}

Character::Character(Type type, const TextureHolder& textures, const FontHolder& fonts)
	: Entity(Table[static_cast<int>(type)].hitpoints)
	, mType(type)
	, mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect)
	, mPlayerAnimation(textures.get(TextureIDs::PlayerMove))
	, mFireCommand()
	, mGrenadeCommand()
	, mFireCountdown(sf::Time::Zero)
	, mIsFiring(false)
	, mIsLaunchingGrenade(false)
	, mShowExplosion(true)
	, mPlayedExplosionSound(false)
	, mSpawnedPickup(false)
	, mFireRateLevel(1)
	, mSpreadLevel(1)
	, mGrenadeAmmo(100)
	, mDropPickupCommand()
	, mTravelledDistance(0.f)
	, mDirectionIndex(0)
	, mHealthDisplay(nullptr)
	, mGrenadeDisplay(nullptr)
	, mGrenadePower(nullptr)
{
	mPlayerAnimation.setFrameSize(sf::Vector2i(263, 156));
	mPlayerAnimation.setNumFrames(18);
	mPlayerAnimation.setScale(0.5f, 0.5f);
	mPlayerAnimation.setDuration(sf::seconds(1));
	mPlayerAnimation.setRepeating(true);
	mPlayerAnimation.setTextureRect();
	
	centreOrigin(mSprite);
	centreOrigin(mPlayerAnimation);

	mFireCommand.category = static_cast<int>(Category::SceneAirLayer);
	mFireCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createBullets(node, textures);
	};

	mGrenadeCommand.category = static_cast<int>(Category::SceneAirLayer);
	mGrenadeCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createProjectile(node, Projectile::ProjectileIDs::Grenade, 0.f, 0.5f, textures);
		mGrenadeVelocity = 0.f;
	};
	

	mDropPickupCommand.category = static_cast<int>(Category::SceneAirLayer);
	mDropPickupCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};

	std::unique_ptr<ShapeNode> healthDisplay(new ShapeNode(sf::Color::Green));
	mHealthDisplay = healthDisplay.get();
	attachChild(std::move(healthDisplay));

	if (getCategory() == static_cast<int>(Category::PlayerCharacter))
	{
		std::unique_ptr<TextNode> grenadeDisplay(new TextNode(fonts, ""));
		grenadeDisplay->setPosition(0, 70);
		mGrenadeDisplay = grenadeDisplay.get();
		attachChild(std::move(grenadeDisplay));

		std::unique_ptr<ShapeNode> grenadePower(new ShapeNode(sf::Color(BLUE)));
		grenadePower->setPosition(0, 70);
		mGrenadePower = grenadePower.get();
		attachChild(std::move(grenadePower));
	}

	updateTexts();
}

void Character::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mPlayerAnimation, states);
}

void Character::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	//Update Player Animations
	updateAnimations(dt);

	// Check if bullets or grenades are fired
	checkProjectileLaunch(dt, commands);

	// Update enemy movement pattern; apply velocity
	//updateMovementPattern(dt);
	

	setRotation(Entity::getAngle() * dt.asSeconds()); // Alex - update players current rotation

	Entity::updateCurrent(dt, commands);

	updateTexts();
}

void Character::updateVelocity(sf::Time dt)//TO DELETE//TODO
{
	rotate(getVelocity().x * dt.asSeconds());
	setVelocity((cos((getRotation()) * M_PI / 180) * -getVelocity().y), (sin((getRotation())* M_PI / 180)* -getVelocity().y));
}

void Character::updateAnimations(sf::Time dt)
{
	if (getVelocity().x != 0.f || getVelocity().y != 0.f)
	{
		mPlayerAnimation.update(dt);
	}
}

unsigned int Character::getCategory() const
{
	if (isAllied())
		return static_cast<int>(Category::PlayerCharacter);
	else
		return static_cast<int>(Category::EnemyCharacter);
}

sf::FloatRect Character::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

bool Character::isMarkedForRemoval() const
{
	return isDestroyed() && (mPlayerAnimation.isFinished() || !mShowExplosion);
}

void Character::remove()
{
	Entity::remove();
	mShowExplosion = false;
}

bool Character::isAllied() const
{
	return mType == Type::Player;
}

float Character::getMaxSpeed() const
{
	return Table[static_cast<int>(mType)].speed;
}

void Character::increaseFireRate()
{
	if (mFireRateLevel < 10)
		++mFireRateLevel;
}

void Character::increaseSpread()
{
	if (mSpreadLevel < 3)
		++mSpreadLevel;
}

void Character::collectGrenades(unsigned int count)
{
	mGrenadeAmmo += count;
}

void Character::fire()
{
	// Only ships with fire interval != 0 are able to fire
	if (Table[static_cast<int>(mType)].fireInterval != sf::Time::Zero)
		mIsFiring = true;
}

void Character::startGrenade()
{
	if (mGrenadeAmmo > 0)
	{
		mGrenadeVelocity = 0;
		mGrenadeStarted = true;
	}
}

void Character::launchGrenade()
{
	if (mGrenadeStarted)
	{

		mIsLaunchingGrenade = true;
		--mGrenadeAmmo;
	}
}

void Character::updateMovementPattern(sf::Time dt)
{
	// Enemy airplane: Movement pattern
	const std::vector<Direction>& directions = Table[static_cast<int>(mType)].directions;
	if (!directions.empty())
	{
		// Moved long enough in current direction: Change direction
		if (mTravelledDistance > directions[mDirectionIndex].distance)
		{
			mDirectionIndex = (mDirectionIndex + 1) % directions.size();
			mTravelledDistance = 0.f;
		}

		// Compute velocity from direction
		float radians = toRadians(directions[mDirectionIndex].angle + 90.f);
		float vx = getMaxSpeed() * std::cos(radians);
		float vy = getMaxSpeed() * std::sin(radians);

		setVelocity(vx, vy);

		mTravelledDistance += getMaxSpeed() * dt.asSeconds();
	}
}

void Character::checkPickupDrop(CommandQueue& commands)
{
	if (!isAllied() && randomInt(3) == 0 && !mSpawnedPickup)
		commands.push(mDropPickupCommand);

	mSpawnedPickup = true;
}

void Character::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	if (mGrenadeStarted)
	{
		clamp(mGrenadeVelocity += (dt.asMilliseconds()), 0, 500.f);
	}

	// Enemies try to fire all the time
	if (!isAllied())
		fire();

	// Check for automatic gunfire, allow only in intervals
	if (mIsFiring && mFireCountdown <= sf::Time::Zero)
	{
		// Interval expired: We can fire a new bullet
		commands.push(mFireCommand);
		playLocalSound(commands, isAllied() ? SoundEffectIDs::AlliedGunfire : SoundEffectIDs::EnemyGunfire);
		mFireCountdown += Table[static_cast<int>(mType)].fireInterval / (mFireRateLevel + 1.f);
		mIsFiring = false;
	}
	else if (mFireCountdown > sf::Time::Zero)
	{
		// Interval not expired: Decrease it further
		mFireCountdown -= dt;
		mIsFiring = false;
	}

	// Check for grenade launch
	if (mIsLaunchingGrenade)
	{
		commands.push(mGrenadeCommand);
		//playLocalSound(commands, SoundEffectIDs::LaunchGrenade);
		mIsLaunchingGrenade = false;
		mGrenadeStarted = false;
	}
}

void Character::createBullets(SceneNode& node, const TextureHolder& textures) const
{
	Projectile::ProjectileIDs type = isAllied() ? Projectile::ProjectileIDs::AlliedBullet : Projectile::ProjectileIDs::EnemyBullet;

	switch (mSpreadLevel)
	{
	case 1:
		createProjectile(node, type, -0.09f, 0.5f, textures);
		break;

	case 2:
		createProjectile(node, type, +0.33f, 0.33f, textures);
		createProjectile(node, type, +0.33f, 0.33f, textures);
		break;

	case 3:
		createProjectile(node, type, -0.5f, 0.33f, textures);
		createProjectile(node, type, 0.0f, 0.5f, textures);
		createProjectile(node, type, +0.5f, 0.33f, textures);
		break;
	}
}

void Character::createProjectile(SceneNode& node, Projectile::ProjectileIDs type, float xOffset, float yOffset, const TextureHolder& textures) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));
	sf::Vector2f offset(xOffset * mSprite.getGlobalBounds().width, yOffset * mSprite.getGlobalBounds().height);

	projectile->setOrigin(offset);
	projectile->setPosition(getWorldPosition());
	projectile->setRotation(getRotation() + 90);

	float radians = toRadians(getRotation());
	float speed = projectile->getMaxSpeed(mGrenadeVelocity);

	float xVelocity = cos(radians) * speed;
	float yVelocity = sin(radians) * speed;
		
	projectile->setVelocity(xVelocity, yVelocity);

	node.attachChild(std::move(projectile));
	
}


void Character::createPickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Pickup::PickupID>(randomInt(static_cast<int>(Pickup::PickupID::TypeCount)));

	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(getWorldPosition());
	pickup->setVelocity(0.f, 1.f);
	node.attachChild(std::move(pickup));
}

void Character::updateTexts()
{
	//mHealthDisplay->setString(std::to_string(getHitpoints()) + " HP");
	mHealthDisplay->setPosition(0.f, 0.f);
	mHealthDisplay->setRotation(-getRotation());
	mHealthDisplay->setOrigin(30.0f, -90.f);
	mHealthDisplay->setSize(getHitpoints(), 5.f);

	mGrenadePower->setPosition(0.f, 0.f);
	mGrenadePower->setRotation(-getRotation());
	mGrenadePower->setOrigin(30.0f, -100.f);

	mGrenadePower->setSize(((mGrenadeVelocity != 0) ? (mGrenadeVelocity / 5) : 0), ((mGrenadeVelocity != 0) ? 5.f : 0));

	if (mGrenadeDisplay)
	{
		mGrenadeDisplay->setPosition(0.f, 0.f);
		mGrenadeDisplay->setRotation(-getRotation());
		mGrenadeDisplay->setOrigin(50.f, -100.f);
		mGrenadeDisplay->setString(std::to_string(mGrenadeAmmo));
	}

	if (getHitpoints() <= 20)
	{
		mHealthDisplay->setFillColor(sf::Color(RED));
	}
	else if (getHitpoints() <= 20)
	{
		mHealthDisplay->setFillColor(sf::Color(ORANGE));
	}
	else
	{
		mHealthDisplay->setFillColor(sf::Color(GREEN));
	}
}


void Character::playLocalSound(CommandQueue& commands, SoundEffectIDs effect)
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

// Alex - Get maximum rotation speed (Set in Constants.hpp & DataTables.cpp)
float Character::getMaxRotationSpeed() const
{
	return Table[static_cast<int>(mType)].rotationSpeed;
}