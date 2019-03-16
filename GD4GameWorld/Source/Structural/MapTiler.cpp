#include "Structural/MapTiler.hpp"
#include "Structural/ResourceHolder.hpp"
#include "DataTable/DataTables.hpp"


//Mike Map Tiler Class, following tutorials from below

//SFML VERTEX ARRAYS
//https://www.sfml-dev.org/tutorials/2.2/graphics-vertex-array.php

//Loading Map From Txt FIle
//https://www.youtube.com/watch?v=NdOY8kuk4Ds&index=30&list=PLHJE4y54mpC5j_x90UkuoMZOdmmL9-_rg

namespace
{
	const std::vector<MapTileData> Table = initializeMapTileData();
}

MapTiler::MapTiler(MapID type, TextureHolder& textures)
	: mMapFile(Table[static_cast<int>(type)].mapFile)
	  , mTexture(textures.get(Table[static_cast<int>(type)].texture))
	  , mTileSize(Table[static_cast<int>(type)].tileSize)
	  , mMapWidth(0)
	  , mMapHeight(-1)
{

	if (mMapFile.is_open())
	{
		populateMap();
		loadMap();
	}
}


void MapTiler::populateMap()
{
	while (!mMapFile.eof())
	{
		populateLine();
	}
}

void MapTiler::populateLine()
{
	std::string line, value;
	//Retrieve line from txt file
	std::getline(mMapFile, line);
	//set contents of buffer
	std::stringstream stream(line);

	//set width of line to 0
	unsigned int width = 0;

	//delimits lines by space
	while (std::getline(stream, value, ' '))
	{
		//ignore trailing spaces
		if (value.length() > 0)
		{
			//count characters for map width
			++width;

			unsigned int x, i;

			findCharacter(i, value);

			//if i is the length of value convert it to int value, else set it to -1
			x = i == value.length() ? atoi(value.c_str()) : -1;

			// add value to array
			tempMap.push_back(x);
		}
	}

	//sets the map dimensions to character count of line for width and line count for height
	setMapDimensions(width);

	//Add Line from Txt File to array
	intMap.insert(intMap.end(), tempMap.begin(), tempMap.end());
	//clear the tempMap
	tempMap.clear();
}

void MapTiler::findCharacter(unsigned int& i, std::string line)
{
	// will return when it finds a character that is not a number
	//if all are numbers by the end of loop, i will == length of the line
	for (i = 0; i < line.length(); i++)
	{
		if (!isdigit(line[i]))
		{
			break;
		}
	}
}

bool MapTiler::loadMap()
{
	// resize the vertex array to fit the level size
	m_vertices.setPrimitiveType(sf::Quads);
	m_vertices.resize(mMapWidth * mMapHeight * 4);

	//populate the vertex array, with one quad per tile
	for (unsigned int i = 0; i < mMapWidth; ++i)
		for (unsigned int j = 0; j < mMapHeight; ++j)
		{
			// get the current tile number
			int tileNumber = intMap[i + j * mMapWidth];
			if (tileNumber != -1)
			{
				// find its position in the tile set texture
				int tu = tileNumber % (mTexture.getSize().x / mTileSize.x);
				int tv = tileNumber / (mTexture.getSize().x / mTileSize.x);

				// get a pointer to the current tile's quad
				sf::Vertex* quad = &m_vertices[(i + j * mMapWidth) * 4];

				// define its 4 corners
				quad[0].position = sf::Vector2f(i * mTileSize.x, j * mTileSize.y);
				quad[1].position = sf::Vector2f((i + 1) * mTileSize.x, j * mTileSize.y);
				quad[2].position = sf::Vector2f((i + 1) * mTileSize.x, (j + 1) * mTileSize.y);
				quad[3].position = sf::Vector2f(i * mTileSize.x, (j + 1) * mTileSize.y);

				// define its 4 texture coordinates
				quad[0].texCoords = sf::Vector2f(tu * mTileSize.x, tv * mTileSize.y);
				quad[1].texCoords = sf::Vector2f((tu + 1) * mTileSize.x, tv * mTileSize.y);
				quad[2].texCoords = sf::Vector2f((tu + 1) * mTileSize.x, (tv + 1) * mTileSize.y);
				quad[3].texCoords = sf::Vector2f(tu * mTileSize.x, (tv + 1) * mTileSize.y);
			}
		}

	return true;
}

void MapTiler::setMapDimensions(unsigned int width)
{
	//sets width of line to largest value
	if (width > mMapWidth)
		mMapWidth = width;

	//Counts the lines to set height
	++mMapHeight;
}

sf::FloatRect MapTiler::getMapBounds()
{
	float pixelWidth = mMapWidth * mTileSize.x - mTileSize.x;
	float pixelHeight = mMapHeight * mTileSize.y - mTileSize.y;

	return sf::FloatRect(0, 0, pixelWidth, pixelHeight);
}

sf::Vector2i MapTiler::getTileSize()
{
	return mTileSize;
}

void MapTiler::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// apply the transform
	states.transform *= getTransform();

	// apply the tile set texture
	states.texture = &mTexture;

	// draw the vertex array
	target.draw(m_vertices, states);
}
