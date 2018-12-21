#include "MapTiler.hpp"
#include "ResourceHolder.hpp"
#include "SpriteNode.hpp"
#include <array>


MapTiler::MapTiler()//(TextureHolder& textures)
	:mMapFile("map.txt")
	//, mTexture(textures.get(TextureIDs::MapTiles))
{
	mTileSize = sf::Vector2u(32, 32);

	mMapWidth = 0;
	mMapHeight = -1;
	mTexture.loadFromFile("colors.png");

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
	std::getline(mMapFile, line);
	std::stringstream stream(line);

	int width = 0;

	while (std::getline(stream, value, ' '))
	{
		//ignore trailing spaces
		if (value.length() > 0)
		{
			++width;
			std::string xx = value.substr(0, value.find(','));

			int x, i;

			findCharacter(i, xx);

			x = (i == xx.length()) ? atoi(xx.c_str()) : -1;

			tempMap.push_back(x);
		}
	}

	setMapDimensions(width);

	//Add Line in Txt File
	intMap.insert(intMap.end(), tempMap.begin(), tempMap.end());

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
			if (tileNumber != -1 )
			{
				// find its position in the tileset texture
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

//std::vector<std::vector<sf::Vector2i>> MapTiler::getMap()
//{
//	return map;
//}

//int MapTiler::getTileSize()
//{
//	return mTileSize;
//}

void MapTiler::setMapDimensions(int width)
{
	if (width > mMapWidth)
		mMapWidth = width;

	++mMapHeight;
}
void MapTiler::draw(sf::RenderTarget & target, sf::RenderStates states) const
{

	// apply the transform
	states.transform *= getTransform();

	// apply the tileset texture
	states.texture = &mTexture;

	// draw the vertex array
	target.draw(m_vertices, states);
}

