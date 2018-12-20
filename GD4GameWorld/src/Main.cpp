//#include "Application.hpp"
//
//#include <stdexcept>
//#include <iostream>

#include "MapTiler.hpp"

int main()
{


	try
	{
		MapTiler mapTiler;
		//mapTiler.drawMap();
		//mapTiler.renderToWindow();
		//Application app;
		//app.run();
	}
	catch (std::exception e)
	{
		std::cout << "\n EXCEPTION: " << e.what() << std::endl;
	}
	//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX



	//std::ifstream mapFile("map.txt");
	//sf::Texture tileTexture;
	//sf::Sprite tiles;

	//std::vector<std::vector<sf::Vector2i>> map;
	//std::vector<sf::Vector2i> tempMap;

	//if (mapFile.is_open())
	//{
	//	std::string tileLocation;
	//	mapFile >> tileLocation;

	//	tileTexture.loadFromFile(tileLocation);

	//	tiles.setTexture(tileTexture);

	//	while (!mapFile.eof())
	//	{
	//		std::string str, value;
	//		std::getline(mapFile, str);
	//		std::stringstream stream(str);

	//		while (std::getline(stream, value, ' '))
	//		{
	//			//ignore trailing spaces
	//			if (value.length() > 0)
	//			{
	//				std::string xx = value.substr(0, value.find(','));
	//				std::string yy = value.substr(value.find(',') + 1);

	//				int x, y, i, j;

	//				for (i = 0; i < xx.length(); i++)
	//				{
	//					if (!isdigit(xx[i]))
	//					{
	//						break;
	//					}
	//				}

	//				for (j = 0; j < yy.length(); j++)
	//				{
	//					if (!isdigit(yy[j]))
	//					{
	//						break;
	//					}
	//				}

	//				x = (i == xx.length()) ? atoi(xx.c_str()) : -1;
	//				y = (j == yy.length()) ? atoi(yy.c_str()) : -1;

	//				tempMap.push_back(sf::Vector2i(x, y));
	//			}
	//		}
	//			map.push_back(tempMap);
	//			tempMap.clear();
	//	}
	//}

	//sf::RenderWindow Window(sf::VideoMode(640, 480, 32), "Loading Map");

	//while (Window.isOpen())
	//{
	//	sf::Event event;

	//	while (Window.pollEvent(event))
	//	{
	//		switch (event.type)
	//		{
	//		case sf::Event::Closed:
	//			Window.close();
	//			break;
	//		}
	//	}
	//	Window.clear(sf::Color::White);
	//	

	//	for (int i = 0; i < map.size(); i++)
	//	{
	//		for (int j = 0; j < map[i].size(); j++)
	//		{
	//			if (map[i][j].x != -1 && map[i][j].y != -1)
	//			{
	//				tiles.setPosition(j * 32, i * 32);
	//				tiles.setTextureRect(sf::IntRect(map[i][j].x * 32, map[i][j].y * 33, 32, 32));
	//				Window.draw(tiles);
	//			}
	//		}
	//	}
	//	Window.display();
	//}

}