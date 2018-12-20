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
#include "World.hpp"

class MapTiler
{

public:
	MapTiler(TextureHolder& textures);
	void drawMap();
	void populateMap();
	void findCharacter(int & i, std::string line);

	std::vector<std::vector<sf::Vector2i>> getMap();
	int getTileSize();
private:
	std::ifstream mMapFile;
	sf::Texture mTexture;
	sf::Sprite mTile;

	std::vector<std::vector<sf::Vector2i>> map;
	std::vector<sf::Vector2i> tempMap;

	int mTileSize;

};