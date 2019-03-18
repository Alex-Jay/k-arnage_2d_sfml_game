#include "Constant/Constants.hpp"
#include "Structural/Utility.hpp"
#include "Animation/Animation.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <cmath>
#include <random>
#include <cassert>
#include <iostream>
#include <Windows.h>
#include <ctime>
#include <list>
#include <sstream>


std::default_random_engine createRandomEngine()
{
	auto seed = static_cast<unsigned long>(std::time(nullptr));
	return std::default_random_engine(seed);
}

auto RandomEngine = createRandomEngine();

void centreOrigin(sf::RectangleShape shape)
{
	sf::FloatRect bounds = shape.getLocalBounds();
	shape.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centerOrigin(sf::Sprite& sprite)
{
	sf::FloatRect bounds = sprite.getLocalBounds();
	sprite.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centreOrigin(sf::Sprite& sprite)
{
	sf::FloatRect bounds = sprite.getLocalBounds();
	sprite.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centerOrigin(sf::Text& text)
{
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centerOrigin(Animation& animation)
{
	sf::FloatRect bounds = animation.getLocalBounds();
	animation.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centreOrigin(Animation& animation)
{
	sf::FloatRect bounds = animation.getLocalBounds();
	animation.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

float toRadians(float degrees)
{
	return degrees * M_PI / 180.f;
}

float toDegrees(float radians)
{
	return radians * 180.f / M_PI;
}

int randomInt(int exclusiveMax)
{
	std::uniform_int_distribution<> distr(0, exclusiveMax - 1);
	return distr(RandomEngine);
}

//Mike
int randomIntExcluding(int lower, int upper)
{
	int isSigned = 1;
	//Random number between 0 and total range
	int rand = randomInt(lower + upper);
	
	if (lower <= 0)
		isSigned = randomInt(3);

	// if the random number is greater than the lowest and the range to push outside.
	if (rand >= lower)
		if (isSigned == 1)
			rand += upper;
		else
			rand -= upper;

	return rand;
}

sf::Vector2f unitVector(sf::Vector2f vector)
{
	assert(vector != sf::Vector2f(0.f, 0.f));
	return vector / length(vector);
}

sf::Vector2f normalize(const sf::Vector2f& source)
{
	float length = sqrt(source.x * source.x + source.y * source.y);
	if (length != 0)
		return sf::Vector2f(source.x / length, source.y / length);
	return source;
}

float length(sf::Vector2f vector)
{
	return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

float vectorDistance(sf::Vector2f v1, sf::Vector2f v2)
{
	sf::Vector2f dV = v1 - v2;
	return length(dV);
}

// Alex - TODO implement simple lerp
sf::Vector2f lerp(sf::Vector2f start, sf::Vector2f end, float duration)
{
	/*
		StartX = 5
		EndX   = 10
		Duration = 5

		dX = 5
		dX / duration = 1s per step
	*/

	sf::Clock clock;
	clock.restart();

	return sf::Vector2f();
}

//Mike
sf::Vector2f MoveTowards(sf::Vector2f current, sf::Vector2f target, float maxDelta)
{
	sf::Vector2f a = target - current;

	float magnitude = length(a);

	if (magnitude <= maxDelta || magnitude == 0.f)
	{
		return target;
	}
	return current + a / magnitude * maxDelta;
}

// Alex - Go to specific coordinate in console
void GotoXY(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//Mike
void clamp(float& value, float min, float max)
{
	value = std::max(min, std::min(value, max));
}

//Mike
sf::FloatRect shrink(int amount, sf::FloatRect& rect)
{
	return sf::FloatRect(rect.left + amount, rect.top + amount, rect.width - amount, rect.height - amount);
}

//Mike
//Returns if a list of rects intersects with one
bool containsIntersection(std::list<sf::FloatRect>& rectList, sf::FloatRect& rect)
{
	for (auto r : rectList)
	{
		if (r.intersects(rect))
		{
			return true;
		}
	}
	return false;
}

sf::Vector2f getCenter(sf::FloatRect& rect)
{
	float x = rect.left + (rect.width / 2);
	float y = rect.top + (rect.height / 2);

	return  sf::Vector2f(x, y);
}

std::string toString(sf::Keyboard::Key key)
{
#define BOOK_KEYTOSTRING_CASE(KEY) case sf::Keyboard::KEY: return #KEY;

	switch (key)
	{
		BOOK_KEYTOSTRING_CASE(Unknown)
			BOOK_KEYTOSTRING_CASE(A)
			BOOK_KEYTOSTRING_CASE(B)
			BOOK_KEYTOSTRING_CASE(C)
			BOOK_KEYTOSTRING_CASE(D)
			BOOK_KEYTOSTRING_CASE(E)
			BOOK_KEYTOSTRING_CASE(F)
			BOOK_KEYTOSTRING_CASE(G)
			BOOK_KEYTOSTRING_CASE(H)
			BOOK_KEYTOSTRING_CASE(I)
			BOOK_KEYTOSTRING_CASE(J)
			BOOK_KEYTOSTRING_CASE(K)
			BOOK_KEYTOSTRING_CASE(L)
			BOOK_KEYTOSTRING_CASE(M)
			BOOK_KEYTOSTRING_CASE(N)
			BOOK_KEYTOSTRING_CASE(O)
			BOOK_KEYTOSTRING_CASE(P)
			BOOK_KEYTOSTRING_CASE(Q)
			BOOK_KEYTOSTRING_CASE(R)
			BOOK_KEYTOSTRING_CASE(S)
			BOOK_KEYTOSTRING_CASE(T)
			BOOK_KEYTOSTRING_CASE(U)
			BOOK_KEYTOSTRING_CASE(V)
			BOOK_KEYTOSTRING_CASE(W)
			BOOK_KEYTOSTRING_CASE(X)
			BOOK_KEYTOSTRING_CASE(Y)
			BOOK_KEYTOSTRING_CASE(Z)
			BOOK_KEYTOSTRING_CASE(Num0)
			BOOK_KEYTOSTRING_CASE(Num1)
			BOOK_KEYTOSTRING_CASE(Num2)
			BOOK_KEYTOSTRING_CASE(Num3)
			BOOK_KEYTOSTRING_CASE(Num4)
			BOOK_KEYTOSTRING_CASE(Num5)
			BOOK_KEYTOSTRING_CASE(Num6)
			BOOK_KEYTOSTRING_CASE(Num7)
			BOOK_KEYTOSTRING_CASE(Num8)
			BOOK_KEYTOSTRING_CASE(Num9)
			BOOK_KEYTOSTRING_CASE(Escape)
			BOOK_KEYTOSTRING_CASE(LControl)
			BOOK_KEYTOSTRING_CASE(LShift)
			BOOK_KEYTOSTRING_CASE(LAlt)
			BOOK_KEYTOSTRING_CASE(LSystem)
			BOOK_KEYTOSTRING_CASE(RControl)
			BOOK_KEYTOSTRING_CASE(RShift)
			BOOK_KEYTOSTRING_CASE(RAlt)
			BOOK_KEYTOSTRING_CASE(RSystem)
			BOOK_KEYTOSTRING_CASE(Menu)
			BOOK_KEYTOSTRING_CASE(LBracket)
			BOOK_KEYTOSTRING_CASE(RBracket)
			BOOK_KEYTOSTRING_CASE(SemiColon)
			BOOK_KEYTOSTRING_CASE(Comma)
			BOOK_KEYTOSTRING_CASE(Period)
			BOOK_KEYTOSTRING_CASE(Quote)
			BOOK_KEYTOSTRING_CASE(Slash)
			BOOK_KEYTOSTRING_CASE(BackSlash)
			BOOK_KEYTOSTRING_CASE(Tilde)
			BOOK_KEYTOSTRING_CASE(Equal)
			BOOK_KEYTOSTRING_CASE(Dash)
			BOOK_KEYTOSTRING_CASE(Space)
			BOOK_KEYTOSTRING_CASE(Return)
			BOOK_KEYTOSTRING_CASE(BackSpace)
			BOOK_KEYTOSTRING_CASE(Tab)
			BOOK_KEYTOSTRING_CASE(PageUp)
			BOOK_KEYTOSTRING_CASE(PageDown)
			BOOK_KEYTOSTRING_CASE(End)
			BOOK_KEYTOSTRING_CASE(Home)
			BOOK_KEYTOSTRING_CASE(Insert)
			BOOK_KEYTOSTRING_CASE(Delete)
			BOOK_KEYTOSTRING_CASE(Add)
			BOOK_KEYTOSTRING_CASE(Subtract)
			BOOK_KEYTOSTRING_CASE(Multiply)
			BOOK_KEYTOSTRING_CASE(Divide)
			BOOK_KEYTOSTRING_CASE(Left)
			BOOK_KEYTOSTRING_CASE(Right)
			BOOK_KEYTOSTRING_CASE(Up)
			BOOK_KEYTOSTRING_CASE(Down)
			BOOK_KEYTOSTRING_CASE(Numpad0)
			BOOK_KEYTOSTRING_CASE(Numpad1)
			BOOK_KEYTOSTRING_CASE(Numpad2)
			BOOK_KEYTOSTRING_CASE(Numpad3)
			BOOK_KEYTOSTRING_CASE(Numpad4)
			BOOK_KEYTOSTRING_CASE(Numpad5)
			BOOK_KEYTOSTRING_CASE(Numpad6)
			BOOK_KEYTOSTRING_CASE(Numpad7)
			BOOK_KEYTOSTRING_CASE(Numpad8)
			BOOK_KEYTOSTRING_CASE(Numpad9)
			BOOK_KEYTOSTRING_CASE(F1)
			BOOK_KEYTOSTRING_CASE(F2)
			BOOK_KEYTOSTRING_CASE(F3)
			BOOK_KEYTOSTRING_CASE(F4)
			BOOK_KEYTOSTRING_CASE(F5)
			BOOK_KEYTOSTRING_CASE(F6)
			BOOK_KEYTOSTRING_CASE(F7)
			BOOK_KEYTOSTRING_CASE(F8)
			BOOK_KEYTOSTRING_CASE(F9)
			BOOK_KEYTOSTRING_CASE(F10)
			BOOK_KEYTOSTRING_CASE(F11)
			BOOK_KEYTOSTRING_CASE(F12)
			BOOK_KEYTOSTRING_CASE(F13)
			BOOK_KEYTOSTRING_CASE(F14)
			BOOK_KEYTOSTRING_CASE(F15)
			BOOK_KEYTOSTRING_CASE(Pause)
	}

	return "";
}
