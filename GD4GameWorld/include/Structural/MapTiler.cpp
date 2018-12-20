#include "MapTiler.hpp"

MapTiler::MapTiler()
	:mMapFile("map.txt")
{
	mTileWidth = 32;
	mTileHeight = 32;
	if (mMapFile.is_open())
	{
		std::string tileLocation;
		mMapFile >> tileLocation;
		mTexture.loadFromFile(tileLocation);
		mTile.setTexture(mTexture);
		drawMap();
		renderToWindow();
	}
}


void MapTiler::drawMap()
{
	while (!mMapFile.eof())
	{
		populateMap();
	}
}

void MapTiler::populateMap()
{
	std::string line, value;
	std::getline(mMapFile, line);
	std::stringstream stream(line);

	while (std::getline(stream, value, ' '))
	{
		//ignore trailing spaces
		if (value.length() > 0)
		{
			std::string xx = value.substr(0, value.find(','));
			std::string yy = value.substr(value.find(',') + 1);

			int x, y, i, j;

			findCharacter(i, xx);
			findCharacter(j, yy);

			x = (i == xx.length()) ? atoi(xx.c_str()) : -1;
			y = (j == yy.length()) ? atoi(yy.c_str()) : -1;

			tempMap.push_back(sf::Vector2i(x, y));
		}
	}

	//Add Last Line in Txt File
	map.push_back(tempMap);
	tempMap.clear();
}

void MapTiler::findCharacter(int& i, std::string line)
{
	for (i = 0; i < line.length(); i++)
	{
		if (!isdigit(line[i]))
		{
			break;
		}
	}
}

void MapTiler::renderToWindow()
{
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

		Window.clear(sf::Color::White);
		drawTiles(Window);
		Window.display();
	}
}

void MapTiler::drawTiles(sf::RenderWindow & Window)
{
	for (int i = 0; i < map.size(); i++)
	{
		for (int j = 0; j < map[i].size(); j++)
		{
			if (map[i][j].x != -1 && map[i][j].y != -1)
			{
				drawTile(Window, i, j);
			}
		}
	}
}

void MapTiler::drawTile(sf::RenderWindow & Window, int x, int y)
{
	mTile.setPosition(y * mTileHeight, x * mTileWidth);
	mTile.setTextureRect(sf::IntRect(map[x][y].x * mTileWidth, map[x][y].y * mTileHeight, mTileWidth, mTileHeight));
	Window.draw(mTile);
}
