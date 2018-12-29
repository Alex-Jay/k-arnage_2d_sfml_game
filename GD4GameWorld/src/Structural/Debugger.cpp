#include "Debugger.hpp"
#include "Utility.hpp" // Contains Windows API handle to use GotoXY()

Debugger& Debugger::GetInstance()
{
	static Debugger instance;

	return instance;
}

void Debugger::LogVector(const char* text, sf::Vector2f vector, int row)
{
	GotoXY(0, row);
	std::cout << "                                               ";
	GotoXY(0, row);
	std::cout << text << " [X, Y]: " << vector.x << ", " << vector.y;
}

void Debugger::LogInt(const char* text, int integer, int row)
{
	GotoXY(0, row);
	std::cout << "                                               ";
	GotoXY(0, row);
	std::cout << text << " : " << integer;
}

void Debugger::LogText(const char* text, int row)
{
	GotoXY(0, row);
	std::cout << "                                               ";
	GotoXY(0, row);
	std::cout << text;
}

Debugger::Debugger()
{
}
