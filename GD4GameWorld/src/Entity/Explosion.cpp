#include "Explosion.hpp"
#include "DataTables.hpp"
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"
#include "Utility.hpp"
#include "CommandQueue.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
#include "EmitterNode.hpp"

namespace
{
	const std::vector<ExplosionData> Table = initializeExplosionData();
}

//Mike Explosion Class
Explosion::Explosion(ExplosionIDs type, const TextureHolder& textures)
	: Entity(1),
	  mType(type),
	  mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect),
	  mPlayedScreamSound(false),
	  explosionTimerStarted(false)
{
	// Alex - Center explosion sprite
	centreOrigin(mSprite);
	mAnimation.setTexture(textures.get(TextureIDs::Explosion));
	mAnimation.setFrameSize(sf::Vector2i(256, 256));
	mAnimation.setNumFrames(16);
	mAnimation.setDuration(sf::seconds(1));
	centreOrigin(mAnimation);
	mShowExplosion = true;

	std::unique_ptr<EmitterNode> smoke(new EmitterNode(Particle::Type::Smoke));
	smoke->setPosition(0.f, getBoundingRect().height / 2.f);
	attachChild(std::move(smoke));



}

unsigned int Explosion::getCategory() const
{
	return static_cast<int>(Category::Explosion);
}

int Explosion::getDamage() const
{
	return Table[static_cast<int>(mType)].damage;
}

sf::FloatRect Explosion::getBoundingRect() const
{
	//TODO Change to circle collision size of animation texture
	float radius = Table[static_cast<int>(mType)].radius;

	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

bool Explosion::isMarkedForRemoval() const
{
	return isDestroyed() && (mAnimation.isFinished() || !mShowExplosion);
}

void Explosion::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	mAnimation.update(dt);

	if (!mPlayedScreamSound)
	{
		SoundEffectIDs soundEffect = randomInt(2) == 0 ? SoundEffectIDs::Explosion1 : SoundEffectIDs::Explosion2;
		playLocalSound(commands, soundEffect);
		mPlayedScreamSound = true;
	}

	if (mAnimation.isFinished())
	{
		remove();
	}
}

void Explosion::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	//drawBoundingRect(target, states);
	target.draw(mAnimation, states);
}

void Explosion::playLocalSound(CommandQueue& commands, SoundEffectIDs effect)
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

void Explosion::StartTimer(sf::Time dt)
{
	explosionTimerStarted = true;
	mExplosionTimer = dt;
}
