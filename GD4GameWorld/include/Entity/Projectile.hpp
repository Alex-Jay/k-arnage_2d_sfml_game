#pragma once
#include "Entity.hpp"
#include "Explosion.hpp"
#include "ResourceIdentifiers.hpp"
#include "Command.hpp"
#include "CommandQueue.hpp"
#include "SoundNode.hpp"
#include "Animation.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

class Projectile : public Entity
{
public:
	enum class ProjectileIDs{AlliedBullet, EnemyBullet, Grenade, TypeCount};

public:
	Projectile(ProjectileIDs type, const TextureHolder& textures);

	void createPickup(SceneNode & node, const TextureHolder & textures) const;

	void guideTowards(sf::Vector2f position);
	bool isGrenade() const;

	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect() const;
	float getMaxSpeed() const;
	float getMaxSpeed(float initialSpeed) const;
	int getDamage() const;
	void setInitialVelocity(float vel);

	bool isMarkedForRemoval() const;

	void remove();

private:
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
	void createExplosion(SceneNode & node) const;
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

	void StartTimer(sf::Time dt);

	void playLocalSound(CommandQueue & commands, SoundEffectIDs effect);

private:
	ProjectileIDs mType;
	Animation mAnimation;
	Command mExplosionCommand;
	Command mDropPickupCommand;

	sf::Sprite mSprite;
	sf::Vector2f mTargetDirection;
	sf::Time mGrenadeTimer;

	bool mGrenadeTimerStarted;
	bool mShowExplosion;
	bool mPlayedExplosionSound;

	float mInitialVelocity;
};