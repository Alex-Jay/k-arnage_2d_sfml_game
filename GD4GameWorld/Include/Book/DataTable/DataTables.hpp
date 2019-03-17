#ifndef BOOK_DATATABLES_HPP
#define BOOK_DATATABLES_HPP

#include "Structural/ResourceIdentifiers.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <vector>
#include <functional>

class Character;
class Explosion;
class character;

struct Direction {
	Direction(float angle, float distance)
		: angle(angle)
		, distance(distance){}

	float angle;
	float distance;
};

struct CharacterData {
	int hitpoints;
	float speed;
	float rotationSpeed;
	float deathScale;
	float moveScale;
	Textures::ID texture;
	Textures::ID moveAnimation;
	int moveFrames;
	Textures::ID deathAnimation;
	int deathFrames;
	sf::IntRect textureRect;
	sf::IntRect deathRect;
	sf::IntRect moveRect;
	sf::Time fireInterval;
	std::vector<Direction> directions;
};

struct characterData {
	int hitpoints;
	float speed;
	Textures::ID texture;
	sf::IntRect textureRect;
	sf::Time fireInterval;
	std::vector<Direction> directions;
	bool hasRollAnimation;
};

struct ProjectileData {
	int damage;
	float speed;
	float textureScale;
	Textures::ID texture;
	sf::IntRect textureRect;
};

struct PickupData {
	std::function<void(Character&)> action;
	Textures::ID texture;
	sf::IntRect textureRect;
};

struct ParticleData {
	sf::Color color;
	sf::Time lifetime;
};

struct MapTileData {
	std::string mapFile;
	sf::Vector2u tileSize;
	Textures::ID texture;
};

struct ExplosionData {
	int damage;
	float radius;
	int lifeTimeSeconds;
	Textures::ID texture;
	sf::IntRect textureRect;
};

struct ObstacleData {
};

std::vector<characterData> initializecharacterData();
std::vector<ProjectileData> initializeProjectileData();
std::vector<PickupData> initializePickupData();
std::vector<ParticleData> initializeParticleData();
std::vector<MapTileData> initializeMapTileData();

std::vector<CharacterData> initializeCharacterData();
std::vector<ExplosionData> initializeExplosionData();
std::vector<ObstacleData> initializeObstacleData();

#endif // BOOK_DATATABLES_HPP