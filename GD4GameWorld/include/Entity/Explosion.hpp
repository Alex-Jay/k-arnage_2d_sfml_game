#pragma once
#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "Command.hpp"
#include "CommandQueue.hpp"
#include "SoundNode.hpp"
#include "Animation.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"

class Explosion : public Entity
{
public:
	enum class ExplosionIDs { GrenadeExplosion, MineExplosion, TypeCount };

public:
	Explosion(ExplosionIDs type);

	virtual unsigned int getCategory() const;
	int getRadious() const;
	int getDamage() const;
	virtual sf::FloatRect getBoundingRect() const;

private:
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
	void StartTimer(sf::Time dt);

private:
	ExplosionIDs mType;
	bool explosionTimerStarted;
	sf::Time mExplosionTimer;
};