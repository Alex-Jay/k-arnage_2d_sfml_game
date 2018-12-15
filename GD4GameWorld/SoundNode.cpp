#include "SoundNode.hpp"
#include "SoundPlayer.hpp"

SoundNode::SoundNode(SoundPlayer& player)
	:SceneNode()
	, mSounds(player)
{
}

void SoundNode::playSound(SoundEffectIDs sound, sf::Vector2f position)
{
	mSounds.play(sound, position);
}

unsigned int SoundNode::getCategory() const
{
	return static_cast<int>(Category::SoundEffect);
}
