#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

class ShapeNode : public SceneNode
{
public:

	explicit ShapeNode(sf::Color fillColor);
	void setFillColor(sf::Color color);
	void setSize(float x, float y);
private:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::RectangleShape mShape;
};