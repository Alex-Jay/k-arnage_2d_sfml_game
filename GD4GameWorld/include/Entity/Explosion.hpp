#pragma once
#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "SoundNode.hpp"
#include "Animation.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/System/Time.hpp"

class Explosion : public Entity
{
public:
	enum class ExplosionIDs { GrenadeExplosion, MineExplosion, TypeCount };

public:
	Explosion(ExplosionIDs type, const TextureHolder& textures);

	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect() const;

	bool isMarkedForRemoval() const;

	int getDamage() const;


private:
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

	void StartTimer(sf::Time dt);
	void playLocalSound(CommandQueue& commands, SoundEffectIDs effect);

private:
	ExplosionIDs mType;
	Animation mAnimation;
	sf::Sprite mSprite;

	bool mShowExplosion;
	bool mPlayedScreamSound;
	bool explosionTimerStarted;

	sf::Time mExplosionTimer;
};
