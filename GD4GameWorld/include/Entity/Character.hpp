#pragma once
#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "Command.hpp"
#include "Projectile.hpp"
#include "TextNode.hpp"
#include "Animation.hpp"

#include "SFML/Graphics/Sprite.hpp"

class Character : public Entity
{
public:
	enum class Type{Player, Zombie, TypeCount};

public:
	Character(Type type, const TextureHolder& texture, const FontHolder& fonts);
	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect() const;
	virtual void remove();
	virtual bool isMarkedForRemoval() const;
	bool isAllied() const;
	float getMaxSpeed() const;

	void increaseFireRate();
	void increaseSpread();
	void collectGrenades(unsigned int count);

	void fire();
	void startGrenade();
	void launchGrenade();
	void playLocalSound(CommandQueue& commands, SoundEffectIDs effect);

private:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
	void updateVelocity(sf::Time dt);

	void updateMovementPattern(sf::Time dt);
	void checkPickupDrop(CommandQueue& commands);
	void checkProjectileLaunch(sf::Time dt, CommandQueue& commands);

	void createBullets(SceneNode& node, const TextureHolder& textures) const;
	void createProjectile(SceneNode& node, Projectile::ProjectileIDs type, float xOffset, float yOffset, const TextureHolder& textures) const;
	void setProjectileProperties(std::unique_ptr<Projectile> projectile, sf::Vector2f offset, float initialVelocity);

	void createPickup(SceneNode& node, const TextureHolder& textures) const;
	void updateTexts();
	void updateRollAnimation();

private:
	Type mType;
	sf::Sprite mSprite;
	Animation mPlayerAnimation;
	Command mFireCommand;
	Command mGrenadeCommand;
	sf::Time mFireCountdown;
	bool mIsFiring;
	bool mIsLaunchingGrenade;
	bool mGrenadeStarted;
	bool mShowExplosion;
	bool mPlayedExplosionSound;
	bool mSpawnedPickup;

	int mFireRateLevel;
	int mSpreadLevel;
	int mGrenadeAmmo;

	Command mDropPickupCommand;
	float mTravelledDistance;
	std::size_t mDirectionIndex;
	TextNode* mHealthDisplay;
	TextNode* mGrenadeDisplay;
};