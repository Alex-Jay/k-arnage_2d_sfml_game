#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <vector>
#include <sstream>
#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"
#include "World.hpp"

//Mike

class MapTiler : public SceneNode 
{
public:
	//Add more Maps at later stage
	enum class MapID { Dessert, City, Forrest, TypeCount};

public:
	MapTiler(MapID type, TextureHolder& textures);

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

	MapID mType;
};


