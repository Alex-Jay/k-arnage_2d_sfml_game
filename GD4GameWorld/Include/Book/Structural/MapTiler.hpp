#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <vector>
#include <sstream>
#include <Book/SceneNode.hpp>
#include "Structural/ResourceIdentifiers.hpp"
#include "Core/World.hpp"

//Mike

class MapTiler : public SceneNode
{
public:
	//Add more Maps at later stage
	enum class MapID { Dessert, City, Forrest, TypeCount };

public:
	MapTiler(MapID type, TextureHolder& textures);

	void populateMap();
	void populateLine();
	sf::FloatRect getMapBounds();
	sf::Vector2i getTileSize();
	void findCharacter(unsigned int& i, std::string line);
private:
	void setMapDimensions(unsigned int width);
	bool loadMap();
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
	std::ifstream mMapFile;
	sf::Texture mTexture;

	std::vector<int> tempMap;
	std::vector<int> intMap;

	sf::Vector2i mTileSize;
	sf::VertexArray m_vertices;

	unsigned int mMapWidth;
	unsigned int mMapHeight;
};
