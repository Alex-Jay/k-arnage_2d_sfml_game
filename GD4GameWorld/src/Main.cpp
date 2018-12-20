//#include "Application.hpp"
//
//#include <stdexcept>
//#include <iostream>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <windows.h>

int main()
{
	//try
	//{
	//	Application app;
	//	app.run();
	//}
	//catch (std::exception e)
	//{
	//	std::cout << "\n EXCEPTION: " << e.what() << std::endl;
	//}

	std::ifstream mapFile("map.txt");
	//char buf[256];
	//GetCurrentDirectoryA(256, buf);
	//std::cout << std::string(buf) + '\\';
	sf::Texture tileTexture;
	sf::Sprite tiles;

	sf::Vector2i map[100][100];
	sf::Vector2i loadCounter = sf::Vector2i(0, 0);

	if (mapFile.is_open())
	{
		std::string tileLocation;
		mapFile >> tileLocation;

		tileTexture.loadFromFile(tileLocation);

		tiles.setTexture(tileTexture);

		while (!mapFile.eof())
		{
			std::string str;
			mapFile >> str;
			char x = str[0], y = str[2];
			if (!isdigit(x) || !isdigit(y))
			{
				// if value is not a number dont draw anything
				map[loadCounter.x][loadCounter.y] = sf::Vector2i(-1, -1);
			}
			else
			{
				// else set value to value of number, -'0' gets the numerical value
				map[loadCounter.x][loadCounter.y] = sf::Vector2i(x - '0', y - '0');
			}

			//if next character is new line
			if (mapFile.peek() == '\n')
			{
				//Move down to next line and first character
				loadCounter.x = 0;
				loadCounter.y++;
			}
			else
			{
				loadCounter.x++;
			}
		}

		loadCounter.y++;
	}

	sf::RenderWindow Window(sf::VideoMode(640, 480, 32), "Loading Map");

	while (Window.isOpen())
	{
		sf::Event event;

		while (Window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				Window.close();
				break;
			}
		}
		Window.clear();
		

		for (int i = 0; i < loadCounter.x; i++)
		{
			for (int j = 0; j < loadCounter.y; j++)
			{
				if (map[i][j].x != -1 && map[i][j].y != -1)
				{
					tiles.setPosition(i * 32, j * 32);
					tiles.setTextureRect(sf::IntRect(map[i][j].x * 32, map[i][j].y * 33, 32, 32));
					Window.draw(tiles);
				}
			}
		}
		Window.display();
	}

}