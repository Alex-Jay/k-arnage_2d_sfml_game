#pragma once
#include "ResourceIdentifiers.hpp"

#include "SFML/System/Time.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Rect.hpp"

#include <vector>
#include <functional>

#include <string>

class Character;
class Explosion;

struct Direction
{
	Direction(float angle, float distance) : angle(angle), distance(distance)
	{
	}

	float angle;
	float distance;
};

struct CharacterData
{
	int hitpoints;
	float speed;
	float rotationSpeed;
	float deathScale;
	float moveScale;
	TextureIDs texture;
	TextureIDs moveAnimation;
	int moveFrames;
	TextureIDs deathAnimation;
	int deathFrames;
	sf::IntRect textureRect;
	sf::IntRect deathRect;
	sf::IntRect moveRect;
	sf::Time fireInterval;
	std::vector<Direction> directions;
};

struct MapTileData
{
	std::string mapFile;
	sf::Vector2u tileSize;
	TextureIDs texture;
};

struct ProjectileData
{
	int damage;
	float speed;
	float textureScale;
	TextureIDs texture;
	sf::IntRect textureRect;
};

struct ExplosionData
{
	int damage;
	float radious;
	int lifeTimeSeconds;
	TextureIDs texture;
	sf::IntRect textureRect;
};

struct PickupData
{
	std::function<void(Character&)> action;
	TextureIDs texture;
	sf::IntRect textureRect;
};

struct ParticleData
{
	sf::Color color;
	sf::Time lifetime;
};

std::vector<MapTileData> initializeMapTileData();
std::vector<CharacterData> initializeCharacterData();
std::vector<ExplosionData> initializeExplosionData();
std::vector<ProjectileData> initializeProjectileData();
std::vector<PickupData> initializePickupData();
std::vector<ParticleData> initializeParticleData();