#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <windows.h>
#include <vector>
#include <sstream>

#include "ResourceIdentifiers.hpp"

class MapTiler
{

public:
	MapTiler(TextureHolder& textures);
	void drawMap();
	void populateMap();
	void findCharacter(int & i, std::string line);
	void renderToWindow();

	void drawTiles(sf::RenderWindow & Window);

	void drawTile(sf::RenderWindow & Window, int x, int y);

private:
	std::ifstream mMapFile;
	sf::Texture mTexture;
	sf::Sprite mTile;

	std::vector<std::vector<sf::Vector2i>> map;
	std::vector<sf::Vector2i> tempMap;

	int mTileWidth;
	int mTileHeight;

};