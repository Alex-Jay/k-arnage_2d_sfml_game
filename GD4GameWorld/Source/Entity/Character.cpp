#include "Entity/Character.hpp"
#include "DataTable/DataTables.hpp"
#include "Structural/Utility.hpp"
#include "Entity/Pickup.hpp"
#include "Command/CommandQueue.hpp"
#include "Structural/ResourceHolder.hpp"
#include "Node/SoundNode.hpp"

#include "Constant/Constants.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Event.hpp>

#include <cmath>
#include <iostream>

namespace
{
	const std::vector<CharacterData> Table = initializeCharacterData();
}

//Mike Character Class, Modified From Aircraft
Character::Character(Type type, const TextureHolder& textures, const FontHolder& fonts)
	: Entity(Table[static_cast<int>(type)].hitpoints)
	  , mType(type)
	  , mCharacterMoveAnimation(textures.get(Table[static_cast<int>(type)].moveAnimation))
	  , mCharacterDeathAnimation(textures.get(Table[static_cast<int>(type)].deathAnimation))
	  , mHealthDisplay(nullptr)
	  , mGrenadeDisplay(nullptr)
	  , mGrenadePower(nullptr)
	  , mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect)
	  , mFireCountdown(sf::Time::Zero)
	  , mIsFiring(false)
	  , mIsLaunchingGrenade(false)
	  , mShowDeath(true)
	  , mPlayedScreamSound(false)
	  , mFireRateLevel(1)
	  , mSpreadLevel(1)
	  , mGrenadeAmmo(2)
	  , mTravelledDistance(0.f)
	  , mDirectionIndex(0)
	  , mLocalIdentifier(0)
{
	// Alex - Optimize texture rect size for collision detection
	mSprite.setTextureRect(sf::IntRect(0, 0, 80, 70));

	mCharacterMoveAnimation.setFrameSize(Table[static_cast<int>(type)].moveRect);
	mCharacterMoveAnimation.setNumFrames(Table[static_cast<int>(type)].moveFrames);
	mCharacterMoveAnimation.setScale(Table[static_cast<int>(type)].moveScale, Table[static_cast<int>(type)].moveScale);
	mCharacterMoveAnimation.setDuration(sf::seconds(1));
	mCharacterMoveAnimation.setRepeating(true);
	mCharacterMoveAnimation.setTextureRect();

	mCharacterDeathAnimation.setFrameSize(Table[static_cast<int>(type)].deathRect);
	mCharacterDeathAnimation.setNumFrames(Table[static_cast<int>(type)].deathFrames);
	mCharacterDeathAnimation.setScale(Table[static_cast<int>(type)].deathScale,
	                                  Table[static_cast<int>(type)].deathScale);
	mCharacterDeathAnimation.setDuration(sf::seconds(1));

	centreOrigin(mSprite);
	centreOrigin(mCharacterMoveAnimation);
	centreOrigin(mCharacterDeathAnimation);

	mFireCommand.category = static_cast<int>(Category::SceneLayer);
	mFireCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createBullets(node, textures);
	};

	mGrenadeCommand.category = static_cast<int>(Category::SceneLayer);
	mGrenadeCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createProjectile(node, Projectile::ProjectileIDs::Grenade, 0.f, 0.5f, textures, getLocalIdentifier());
		mGrenadeVelocity = 0.f;
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
	// Alex - Debug Bounding Rectangle	
	//drawBoundingRect(target, states);

	if (isDestroyed() && mShowDeath)
	{
		target.draw(mCharacterDeathAnimation, states);
	}
	else
	{
		target.draw(mCharacterMoveAnimation, states);
	}
}

void Character::guideTowards(sf::Vector2f position)
{
	assert(mType == Character::Type::Zombie);
	mTargetDirection = unitVector(position - getWorldPosition());
}

void Character::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Get players position
	mLastPosition = getPosition();

	//Update Player Animations
	updateAnimations(dt, commands);

	// Check if bullets or grenades are fired
	checkProjectileLaunch(dt, commands);

	if(!isDestroyed())
	{
		move(dt, commands);
	}

	updateTexts();
}

void Character::move(sf::Time dt, CommandQueue& commands)
{ 
	if (mType == Type::Zombie)
	{
		sf::Vector2f newVelocity = mTargetDirection * dt.asSeconds();
		newVelocity *= getMaxSpeed() * 50;

		float angle = std::atan2(newVelocity.y, newVelocity.x);
		setRotation(toDegrees(angle));
		setVelocity(newVelocity);
	}
	else if (mType == Type::Player)
	{
		setRotation(Entity::getAngle() * dt.asSeconds() * getMaxRotationSpeed()); // Alex - update players current rotation
		//updateVelocity(dt);
	}

	Entity::updateCurrent(dt, commands);
}

void Character::updateVelocity(sf::Time dt) //TO DELETE//TODO
{
	rotate(getVelocity().x * dt.asSeconds());
	setVelocity(cos(getRotation() * M_PI / 180) * -getVelocity().y, sin(getRotation() * M_PI / 180) * -getVelocity().y);
}

void Character::updateAnimations(sf::Time dt, CommandQueue& commands)
{
	if (isDestroyed())
	{
		mCharacterDeathAnimation.update(dt);

		if (!mPlayedScreamSound && isPlayer())
		{
			SoundEffect::ID soundEffect = SoundEffect::ID::Scream;
			playLocalSound(commands, soundEffect);

			mPlayedScreamSound = true;
		}
		return;
	}
	if (getVelocity().x != 0.f || getVelocity().y != 0.f)
	{
		mCharacterMoveAnimation.update(dt);
	}
}

unsigned int Character::getCategory() const
{
	if (isPlayer())
		return static_cast<int>(Category::PlayerCharacter);
	return static_cast<int>(Category::EnemyCharacter);
}

sf::FloatRect Character::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

bool Character::isMarkedForRemoval() const
{
	return isDestroyed() && (mCharacterDeathAnimation.isFinished() || !mShowDeath);
}

void Character::remove()
{
	Entity::remove();
	mShowDeath = false;
}

bool Character::isPlayer() const
{
	return mType == Type::Player;
}

bool Character::isZombie() const
{
	return mType == Type::Zombie;
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

void Character::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	if (mGrenadeStarted)
	{
		clamp(mGrenadeVelocity += dt.asMilliseconds(), 0, 500.f);
	}

	// Enemies try to fire all the time
	if (!isPlayer())
		fire();

	// Check for automatic gunfire, allow only in intervals
	if (mIsFiring && mFireCountdown <= sf::Time::Zero)
	{
		// Interval expired: We can fire a new bullet
		commands.push(mFireCommand);
		playLocalSound(commands,  SoundEffect::ID::AlliedGunfire);
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
	//TODO ALL Bullets Damage Everybody, NO Enemy or Allied Bullets
	createProjectile(node, Projectile::ProjectileIDs::AlliedBullet, -0.09f, 0.5f, textures, getLocalIdentifier());
}

void Character::createProjectile(SceneNode& node, Projectile::ProjectileIDs type, float xOffset, float yOffset,
                                 const TextureHolder& textures, unsigned int projectileId) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));
	sf::Vector2f offset(xOffset * mSprite.getGlobalBounds().width, yOffset * mSprite.getGlobalBounds().height);

	projectile->setProjectileId(projectileId);
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

void Character::updateTexts()
{
	mHealthDisplay->setPosition(0.f, 0.f);
	mHealthDisplay->setRotation(-getRotation());
	mHealthDisplay->setOrigin(30.0f, -90.f);

	//Sets dimensions of health display to 0x 0y if health is depleted
	mHealthDisplay->setSize(getHitpoints() != 0 ? getHitpoints()  : 0, getHitpoints() != 0 ? 5.f : 0);

	if (mGrenadeDisplay)
	{
		mGrenadePower->setPosition(0.f, 0.f);
		mGrenadePower->setRotation(-getRotation());
		mGrenadePower->setOrigin(30.0f, -100.f);

		mGrenadePower->setSize(mGrenadeVelocity != 0 ? mGrenadeVelocity / 5 : 0, mGrenadeVelocity != 0 ? 5.f : 0);

		mGrenadeDisplay->setPosition(0.f, 0.f);
		mGrenadeDisplay->setRotation(-getRotation());
		mGrenadeDisplay->setOrigin(50.f, -100.f);
		mGrenadeDisplay->setString(std::to_string(mGrenadeAmmo));
	}

	if (getHitpoints() <= 20)
	{
		mHealthDisplay->setFillColor(sf::Color(RED));
	}
	else if (getHitpoints() > 20 && getHitpoints() <= 70)
	{
		mHealthDisplay->setFillColor(sf::Color(ORANGE));
	}
	else if (getHitpoints() > 70)
	{
		mHealthDisplay->setFillColor(sf::Color(GREEN));
	}
}

void Character::playLocalSound(CommandQueue& commands, SoundEffect::ID effect)
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

void Character::setGrenadeAmmo(sf::Int32 ammo)
{
	mGrenadeAmmo = ammo;
}

unsigned int Character::getGrenadeAmmo()
{
	return mGrenadeAmmo;
}

// Alex - Get maximum rotation speed (Set in Constants.hpp & DataTables.cpp)
float Character::getMaxRotationSpeed() const
{
	return Table[static_cast<int>(mType)].rotationSpeed;
}

unsigned int const Character::getLocalIdentifier() const
{
	return mLocalIdentifier;
}

void Character::setLocalIdentifier(unsigned int localID)
{
	mLocalIdentifier = localID;
}

sf::Vector2f Character::getLastPosition()
{
	return mLastPosition;

}

void Character::setLastPosition(sf::Vector2f position)
{
	mLastPosition = position;
}
