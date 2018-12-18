#pragma once
#include "ResourceIdentifiers.hpp"

#include "SFML/System/Time.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Rect.hpp"

#include <vector>
#include <functional>

class Aircraft;
class Character;

struct Direction
{
	Direction(float angle, float distance) : angle(angle), distance(distance)
	{
	}

	float angle;
	float distance;
};

struct AircraftData
{
	int hitpoints;
	float speed;
	TextureIDs texture;
	sf::IntRect textureRect;
	sf::Time fireInterval;
	std::vector<Direction> directions;
	bool hasRollAnimation;
};

struct CharacterData
{
	int hitpoints;
	float speed;
	TextureIDs texture;
	sf::IntRect textureRect;
	sf::Time fireInterval;
	std::vector<Direction> directions;
	bool hasAnimation;
};

struct ProjectileData
{
	int damage;
	float speed;
	TextureIDs texture;
	sf::IntRect textureRect;
};

struct PickupData
{
	std::function<void(Aircraft&)> action;
	TextureIDs texture;
	sf::IntRect textureRect;
};

struct ParticleData
{
	sf::Color color;
	sf::Time lifetime;
};

std::vector<AircraftData> initializeAircraftData();

std::vector<CharacterData> initializeCharacterData();

std::vector<ProjectileData> initializeProjectileData();
std::vector<PickupData> initializePickupData();
std::vector<ParticleData> initializeParticleData();