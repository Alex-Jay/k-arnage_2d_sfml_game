#pragma once
#include "Entity/Entity.hpp"
#include "Structural/ResourceIdentifiers.hpp"
#include "Command/Command.hpp"
#include "Entity/Projectile.hpp"
#include "Node/TextNode.hpp"
#include "Node/ShapeNode.hpp"
#include "Animation/Animation.hpp"

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
	int16_t getMaxRotationSpeed() const;

	void increaseFireRate();
	void increaseSpread();
	void collectGrenades(unsigned int count);
	void guideTowards(sf::Vector2f position);
	void fire();
	void startGrenade();
	void launchGrenade();
	void playLocalSound(CommandQueue& commands, SoundEffect::ID effect);
	void setSpriteColorTint(sf::Color color);

	void setGrenadeAmmo(sf::Int32 ammo);
	sf::Int32 getGrenadeAmmo();



	sf::Vector2f getLastPosition();
	void setLastPosition(sf::Vector2f position);

	unsigned int const getLocalIdentifier() const;
	void setLocalIdentifier(unsigned int localID);
	bool	isShowingBadge;

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
	                      const TextureHolder& textures, unsigned int projectileId) const;

	void updateTexts();

private:
	Type mType;

	Animation mCharacterMoveAnimation;
	Animation mCharacterDeathAnimation;

	Command mFireCommand;
	Command mGrenadeCommand;

	ShapeNode*	mHealthDisplay;
	TextNode*	mGrenadeDisplay;
	TextNode*	mLocalBadge;
	ShapeNode*	mGrenadePower;

	sf::Sprite mSprite;
	sf::Time mFireCountdown;
	sf::Vector2f mTargetDirection;
	sf::Vector2f mLastPosition;

	bool mIsFiring;
	bool mIsLaunchingGrenade;
	bool mGrenadeStarted;
	bool mShowDeath;
	bool mPlayedScreamSound;

	float	mGrenadeVelocity;
	float	mTravelledDistance;

	int8_t mFireRateLevel;
	int8_t mSpreadLevel;
	sf::Int32 mGrenadeAmmo;

	std::size_t mDirectionIndex;

	sf::Int32 mLocalIdentifier;

	bool networkNotified{};

};

