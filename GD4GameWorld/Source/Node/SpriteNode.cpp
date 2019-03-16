#include "Node/SpriteNode.hpp"
#include <SFML/Graphics/RenderTarget.hpp>


SpriteNode::SpriteNode()
{
}


SpriteNode::SpriteNode(const sf::Texture& texture)
: mSprite(texture)
{
}	

SpriteNode::SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect)
: mSprite(texture, textureRect)
{
}

void SpriteNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}

void SpriteNode::setTextureRect(const sf::IntRect& textureRect)
{
	mSprite.setTextureRect(textureRect);
}