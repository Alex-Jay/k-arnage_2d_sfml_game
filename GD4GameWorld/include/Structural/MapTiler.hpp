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

class MapTiler : public sf::Drawable, public sf::Transformable
{

public:
	//MapTiler(TextureHolder& textures);
	MapTiler();

	void populateMap();
	void populateLine();

	void findCharacter(int & i, std::string line);
	//std::vector<std::vector<sf::Vector2i>> getMap();
private:
	void setMapDimensions(int width);
	bool loadMap();
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
private:
	std::ifstream mMapFile;
	sf::Texture mTexture;

	std::vector<int> tempMap;
	std::vector<int> intMap;

	sf::Vector2u mTileSize;
	sf::VertexArray m_vertices;

	int mMapWidth;
	int mMapHeight;
};


