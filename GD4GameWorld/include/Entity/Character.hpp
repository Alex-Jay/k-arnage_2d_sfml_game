#pragma once
#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "Command.hpp"
#include "Projectile.hpp"
#include "TextNode.hpp"
#include "ShapeNode.hpp"
#include "Animation.hpp"

#include "SFML/Graphics/Sprite.hpp"

class Character : public Entity
{
public:
	enum class Type { Player, Zombie, TypeCount };

	Character(Type type, const TextureHolder& texture, const FontHolder& fonts);

	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect() const;
	virtual void remove();
	virtual bool isMarkedForRemoval() const;

	bool isPlayer() const;
	bool isZombie() const;

	float getMaxSpeed() const;
	float getMaxRotationSpeed() const;

	void increaseFireRate();
	void increaseSpread();
	void collectGrenades(unsigned int count);
	void guideTowards(sf::Vector2f position);
	void fire();
	void startGrenade();
	void launchGrenade();
	void playLocalSound(CommandQueue& commands, SoundEffectIDs effect);

	sf::Vector2f getLastPosition();
	void setLastPosition(sf::Vector2f position);

	unsigned int const getLocalIdentifier() const;
	void setLocalIdentifier(unsigned int localID);

private:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);

	void move(sf::Time dt, CommandQueue & commands);

	void updateVelocity(sf::Time dt);
	void updateAnimations(sf::Time dt, CommandQueue& commands);
	void updateMovementPattern(sf::Time dt);
	void checkProjectileLaunch(sf::Time dt, CommandQueue& commands);

	void createBullets(SceneNode& node, const TextureHolder& textures) const;
	void createProjectile(SceneNode& node, Projectile::ProjectileIDs type, float xOffset, float yOffset,
	                      const TextureHolder& textures) const;

	void updateTexts();

private:
	Type mType;

	Animation mCharacterMoveAnimation;
	Animation mCharacterDeathAnimation;

	Command mFireCommand;
	Command mGrenadeCommand;

	ShapeNode* mHealthDisplay;
	TextNode* mGrenadeDisplay;
	ShapeNode* mGrenadePower;

	sf::Sprite mSprite;
	sf::Time mFireCountdown;
	sf::Vector2f mTargetDirection;
	sf::Vector2f mLastPosition;

	bool mIsFiring;
	bool mIsLaunchingGrenade;
	bool mGrenadeStarted;
	bool mShowDeath;
	bool mPlayedScreamSound;

	float mGrenadeVelocity;
	float mTravelledDistance;

	int mFireRateLevel;
	int mSpreadLevel;
	int mGrenadeAmmo;

	std::size_t mDirectionIndex;

	int mLocalIdentifier;

};

