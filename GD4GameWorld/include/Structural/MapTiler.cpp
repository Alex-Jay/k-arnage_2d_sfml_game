#include "MapTiler.hpp"
#include "ResourceHolder.hpp"
#include "SpriteNode.hpp"
#include <array>


MapTiler::MapTiler(TextureHolder& textures)
	:mMapFile("map.txt")
	, mTexture(textures.get(TextureIDs::MapTiles))
{
	mTileSize = 128;
	if (mMapFile.is_open())
	{
		mTile.setTexture(mTexture);
		drawMap();
		//renderToWindow();
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

std::vector<std::vector<sf::Vector2i>> MapTiler::getMap()
{
	return map;
}

int MapTiler::getTileSize()
{
	return mTileSize;
}
