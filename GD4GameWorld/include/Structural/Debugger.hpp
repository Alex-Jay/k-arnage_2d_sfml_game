#pragma once
/*
	Debug class using Singleton pattern.
*/
#include <SFML/System/Vector2.hpp>

#include <iostream>

class Debugger
{
public:

	// Static access method
	static Debugger& GetInstance();

	// Custom copy/move/assignment
	Debugger(const Debugger&) = delete;
	Debugger(Debugger&&) = delete;
	Debugger& operator=(const Debugger&) = delete;
	Debugger& operator=(Debugger&&) = delete;

	void LogVector(const char* text, sf::Vector2f vector, int row);
	void LogInt(const char* text, int integer, int row);
	void LogText(const char* text, int row);

private:
	// Private constructor to prevent instancing
	Debugger();
};
