#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Vector2.hpp>

#include <sstream>
#include <SFML/Graphics/Rect.hpp>
#include <list>

namespace sf
{
	class Sprite;
	class Text;
	class RectangleShape;
	class Time;
}

class Animation;

// Convert enumerators to strings
std::string		toString(sf::Keyboard::Key key);

void centreOrigin(sf::RectangleShape shape);
void			centerOrigin(sf::Sprite& sprite);
void			centreOrigin(sf::Sprite& sprite);
void			centerOrigin(sf::Text& text);
void			centerOrigin(Animation& animation);
void			centreOrigin(Animation& animation);
float toRadians(float degrees);
float toDegrees(float radians);
int randomInt(int range);
int randomIntExcluding(int lower, int upper);
sf::Vector2f unitVector(sf::Vector2f vector);
sf::Vector2f normalize(const sf::Vector2f& source);
sf::Vector2f lerp(sf::Vector2f start, sf::Vector2f end, float duration, sf::Time dt);
float length(sf::Vector2f vector);
float vectorDistance(sf::Vector2f v1, sf::Vector2f v2);
void clamp(float& value, float min, float max);
sf::Vector2f MoveTowards(sf::Vector2f current, sf::Vector2f target, float maxDelta);
void GotoXY(int x, int y);
sf::FloatRect shrink(int amount, sf::FloatRect& rect);
sf::Vector2f getCenter(sf::FloatRect& rect);

bool containsIntersection(std::list<sf::FloatRect>& rectList, sf::FloatRect& rect);
