#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Vector2.hpp>

#include <sstream>

namespace sf 
{
	class Sprite;
	class Text;
}

class Animation;

void centreOrigin(sf::Sprite& sprite);
void centreOrigin(sf::Text& text);
void centreOrigin(Animation& animation);
float toRadians(float degrees);
float toDegrees(float radians);
int randomInt(int range);
sf::Vector2f unitVector(sf::Vector2f vector);
float length(sf::Vector2f vector);

// Convert enumerators to strings
std::string toString(sf::Keyboard::Key key);
