#include "Explosion.hpp"
#include "DataTables.hpp"
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
	const std::vector<ExplosionData> Table = initializeExplosionData();
}

Explosion::Explosion(ExplosionIDs type, const TextureHolder& textures)
	: Entity(1)
	, mType(type)
{
	mAnimation.setTexture(textures.get(TextureIDs::Explosion));
	mAnimation.setFrameSize(sf::Vector2i(256, 256));
	mAnimation.setNumFrames(16);
	mAnimation.setDuration(sf::seconds(1));
	centreOrigin(mAnimation);
	mShowExplosion = true;
}

unsigned int Explosion::getCategory() const
{
	return static_cast<int>(Category::Explosion);
}

int Explosion::getRadious() const
{
	return Table[static_cast<int>(mType)].radious;
}

int Explosion::getDamage() const
{
	return Table[static_cast<int>(mType)].damage;
}

sf::FloatRect Explosion::getBoundingRect() const
{
	float radious = Table[static_cast<int>(mType)].radious;//TODO Change to circle collision size of animation texture
	return getWorldTransform().transformRect(sf::FloatRect(0,0, radious, radious));
}

bool Explosion::isMarkedForRemoval() const
{
	return isDestroyed() && (mAnimation.isFinished() || !mShowExplosion);
}

void Explosion::updateCurrent(sf::Time dt, CommandQueue & commands)
{
	mAnimation.update(dt);

	if (!mPlayedExplosionSound)
	{
		SoundEffectIDs soundEffect = (randomInt(2) == 0) ? SoundEffectIDs::Explosion1 : SoundEffectIDs::Explosion2;
		playLocalSound(commands, soundEffect);
		mPlayedExplosionSound = true;
	}

	if (mAnimation.isFinished())
	{
		remove();
	}
}

void Explosion::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
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