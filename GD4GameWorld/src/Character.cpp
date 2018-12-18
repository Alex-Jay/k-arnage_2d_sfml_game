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
	, mGrenadeAmmo(2)
	, mDropPickupCommand()
	, mTravelledDistance(0.f)
	, mDirectionIndex(0)
	, mHealthDisplay(nullptr)
	, mGrenadeDisplay(nullptr)
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
	};

	mDropPickupCommand.category = static_cast<int>(Category::SceneAirLayer);
	mDropPickupCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};

	std::unique_ptr<TextNode> healthDisplay(new TextNode(fonts, ""));
	mHealthDisplay = healthDisplay.get();
	attachChild(std::move(healthDisplay));

	if (getCategory() == static_cast<int>(Category::PlayerCharacter))
	{
		std::unique_ptr<TextNode> missileDisplay(new TextNode(fonts, ""));
		missileDisplay->setPosition(0, 70);
		mGrenadeDisplay = missileDisplay.get();
		attachChild(std::move(missileDisplay));
	}

	updateTexts();
}

void Character::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	//if (isDestroyed() && mShowExplosion)
	//	target.draw(mPlayerAnimation, states);
	//else
	//	target.draw(mSprite, states);

	target.draw(mPlayerAnimation, states);
}

void Character::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Update texts and roll animation
	//updateTexts();
	updateRollAnimation();

	// Entity has been destroyed: Possibly drop pickup, mark for removal
	//if (isDestroyed())
	//{
	//	checkPickupDrop(commands);
	//	mPlayerAnimation.update(dt);
	//	// Play explosion sound only once
	//	if (!mPlayedExplosionSound)
	//	{
	//		SoundEffectIDs soundEffect = (randomInt(2) == 0) ? SoundEffectIDs::Explosion1 : SoundEffectIDs::Explosion2;
	//		playLocalSound(commands, soundEffect);
	//		mPlayedExplosionSound = true;
	//	}
	//	return;
	//}

	if (getVelocity().x != 0.f || getVelocity().y != 0.f)
	{
		mPlayerAnimation.update(dt);
	}

	// Check if bullets or missiles are fired
	checkProjectileLaunch(dt, commands);

	// Update enemy movement pattern; apply velocity
	updateMovementPattern(dt);
	
	updateVelocity(dt);
	Entity::updateCurrent(dt, commands);

	updateTexts();
}

void Character::updateVelocity(sf::Time dt)
{
	rotate(getVelocity().x * dt.asSeconds());
	setVelocity((cos((getRotation()) * M_PI / 180) * -getVelocity().y), (sin((getRotation())* M_PI / 180)* -getVelocity().y));

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

void Character::launchGrenade()
{
	if (mGrenadeAmmo > 0)
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

	// Check for missile launch
	if (mIsLaunchingGrenade)
	{
		commands.push(mGrenadeCommand);
		playLocalSound(commands, SoundEffectIDs::LaunchGrenade);
		mIsLaunchingGrenade = false;
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
	projectile->setVelocity((cos((getRotation()) * M_PI / 180) * projectile->getMaxSpeed()), (sin((getRotation())* M_PI / 180)* projectile->getMaxSpeed()));

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
	mHealthDisplay->setString(std::to_string(getHitpoints()) + " HP");
	mHealthDisplay->setPosition(0.f, 0.f);
	mHealthDisplay->setRotation(-getRotation());
	mHealthDisplay->setOrigin(0.0f, -100.f);

	if (mGrenadeDisplay)
	{
		mGrenadeDisplay->setPosition(0.f, 0.f);
		mGrenadeDisplay->setRotation(-getRotation());
		mGrenadeDisplay->setOrigin(0.0f, -120.f);
		if (mGrenadeAmmo == 0)
			mGrenadeDisplay->setString("");
		else
			mGrenadeDisplay->setString("M: " + std::to_string(mGrenadeAmmo));
	}

	if (getHitpoints() <= 40)
	{
		mHealthDisplay->setColor(sf::Color::Red);
	}
	else
	{
		mHealthDisplay->setColor(sf::Color::Green);
	}
}

void Character::updateRollAnimation()
{
	//if (Table[static_cast<int>(mType)].hasAnimation)
	//{
	//	sf::IntRect textureRect = Table[static_cast<int>(mType)].textureRect;

	//	// Roll left: Texture rect offset once
	//	if (getVelocity().x < 0.f)
	//		textureRect.left += textureRect.width;

	//	// Roll right: Texture rect offset twice
	//	else if (getVelocity().x > 0.f)
	//		textureRect.left += 2 * textureRect.width;

	//	mSprite.setTextureRect(textureRect);
	//}


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
