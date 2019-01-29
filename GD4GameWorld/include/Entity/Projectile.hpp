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
	enum class ProjectileIDs { AlliedBullet, EnemyBullet, Grenade, TypeCount };

public:
	Projectile(ProjectileIDs type, const TextureHolder& textures);

	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect() const;

	float getMaxSpeed() const;
	float getMaxSpeed(float initialSpeed) const;

	int getDamage() const;

	bool isGrenade() const;
	bool isMarkedForRemoval() const;

	void setInitialVelocity(float vel);
	void remove();

	unsigned int const getProjectileId() const;
	void setProjectileId(unsigned int id);

private:
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

	void handleGrenade(sf::Time dt, CommandQueue& commands);
	void StartTimer(sf::Time dt);
	void createExplosion(SceneNode& node, const TextureHolder& textures) const;

private:
	ProjectileIDs mType;
	Animation mAnimation;
	Command mExplosionCommand;

	sf::Sprite mSprite;
	sf::Vector2f mTargetDirection;
	sf::Time mGrenadeTimer;

	bool mGrenadeTimerStarted;
	bool mShowExplosion;
	bool mPlayedScreamSound;

	float mInitialVelocity;

	unsigned int mProjectileId;
};
