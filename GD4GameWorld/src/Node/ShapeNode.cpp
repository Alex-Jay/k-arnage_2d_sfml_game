#include "ShapeNode.hpp"
#include "Utility.hpp"

#include "SFML/Graphics/RenderTarget.hpp"

ShapeNode::ShapeNode(sf::Color fillColor)
{
	mShape.setFillColor(fillColor);
	mShape.setOutlineColor(sf::Color::Black);
	mShape.setOutlineThickness(1);
	mShape.setSize(sf::Vector2f(0, 0));
	//centreOrigin(mShape);
}

void ShapeNode::setFillColor(sf::Color color)
{
	mShape.setFillColor(color);
}

void ShapeNode::setSize(int x, int y)
{
	mShape.setSize(sf::Vector2f(x, y));
	//Dont center origin as bar will fill from both sides
	//centreOrigin(mShape);
}

void ShapeNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mShape, states);
}
