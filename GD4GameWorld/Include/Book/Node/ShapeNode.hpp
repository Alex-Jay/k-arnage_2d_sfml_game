#pragma once
#include "Structural/ResourceHolder.hpp"
#include "Structural/ResourceIdentifiers.hpp"
#include "Node/SceneNode.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

class ShapeNode : public SceneNode
{
public:

	explicit ShapeNode(sf::Color fillColor);
	void setFillColor(sf::Color color);
	void setSize(int x, int y);
private:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::RectangleShape mShape;
};
