#include "Entity/Character.hpp"
#include "Entity/Aircraft.hpp"
#include "Entity/Projectile.hpp"
#include "Entity/Pickup.hpp"
#include "Entity/Particle.hpp"
#include "Structural/MapTiler.hpp"
#include "Entity/Explosion.hpp"

#include "DataTable/DataTables.hpp"
#include "Constant/Constants.hpp"


// For std::bind() placeholders _1, _2, ...
using namespace std::placeholders;

std::vector<AircraftData> initializeAircraftData()
{
	std::vector<AircraftData> data(Aircraft::TypeCount);

	data[Aircraft::Eagle].hitpoints = 100;
	data[Aircraft::Eagle].speed = 200.f;
	data[Aircraft::Eagle].fireInterval = sf::seconds(1);
	data[Aircraft::Eagle].texture = Textures::Entities;
	data[Aircraft::Eagle].textureRect = sf::IntRect(0, 0, 48, 64);
	data[Aircraft::Eagle].hasRollAnimation = true;

	data[Aircraft::Raptor].hitpoints = 20;
	data[Aircraft::Raptor].speed = 80.f;
	data[Aircraft::Raptor].texture = Textures::Entities;
	data[Aircraft::Raptor].textureRect = sf::IntRect(144, 0, 84, 64);
	data[Aircraft::Raptor].directions.push_back(Direction(+45.f, 80.f));
	data[Aircraft::Raptor].directions.push_back(Direction(-45.f, 160.f));
	data[Aircraft::Raptor].directions.push_back(Direction(+45.f, 80.f));
	data[Aircraft::Raptor].fireInterval = sf::Time::Zero;
	data[Aircraft::Raptor].hasRollAnimation = false;

	data[Aircraft::Avenger].hitpoints = 40;
	data[Aircraft::Avenger].speed = 50.f;
	data[Aircraft::Avenger].texture = Textures::Entities;
	data[Aircraft::Avenger].textureRect = sf::IntRect(228, 0, 60, 59);
	data[Aircraft::Avenger].directions.push_back(Direction(+45.f,  50.f));
	data[Aircraft::Avenger].directions.push_back(Direction(  0.f,  50.f));
	data[Aircraft::Avenger].directions.push_back(Direction(-45.f, 100.f));
	data[Aircraft::Avenger].directions.push_back(Direction(  0.f,  50.f));
	data[Aircraft::Avenger].directions.push_back(Direction(+45.f,  50.f));
	data[Aircraft::Avenger].fireInterval = sf::seconds(2);
	data[Aircraft::Avenger].hasRollAnimation = false;

	return data;
}

std::vector<ProjectileData> initializeProjectileData()
{
	std::vector<ProjectileData> data(static_cast<int>(Projectile::ProjectileIDs::TypeCount));

	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].damage = BULLETDAMAGE;
	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].speed = BULLETSPEED;
	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].texture = Textures::ID::Entities;
	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].textureRect = sf::IntRect(175, 64, 3, 14);
	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].textureScale = 1;

	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].damage = BULLETDAMAGE;
	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].speed = BULLETSPEED;
	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].texture = Textures::ID::Entities;
	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].textureRect = sf::IntRect(178, 64, 3, 14);
	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].textureScale = 1;

	data[static_cast<int>(Projectile::ProjectileIDs::Grenade)].damage = GRENADEDAMAGE;
	data[static_cast<int>(Projectile::ProjectileIDs::Grenade)].speed = GRENADESPEED;
	data[static_cast<int>(Projectile::ProjectileIDs::Grenade)].texture = Textures::ID::Grenade;
	data[static_cast<int>(Projectile::ProjectileIDs::Grenade)].textureRect = sf::IntRect(0, 0, 718, 800);
	data[static_cast<int>(Projectile::ProjectileIDs::Grenade)].textureScale = 0.03f;

	return data;
}

std::vector<PickupData> initializePickupData()
{
	std::vector<PickupData> data(static_cast<int>(Pickup::Type::TypeCount));

	data[static_cast<int>(Pickup::Type::HealthRefill)].texture = Textures::Entities;
	data[static_cast<int>(Pickup::Type::HealthRefill)].textureRect = sf::IntRect(0, 64, 40, 40);
	data[static_cast<int>(Pickup::Type::HealthRefill)].action = [](Character& a)
	{
		a.repair(HEALTHPICKUPREPAIRVALUE);
	};

	data[static_cast<int>(Pickup::Type::GrenadeRefill)].texture = Textures::ID::Entities;
	data[static_cast<int>(Pickup::Type::GrenadeRefill)].textureRect = sf::IntRect(40, 64, 40, 40);
	data[static_cast<int>(Pickup::Type::GrenadeRefill)].action = std::bind(
		&Character::collectGrenades, _1, GRENADEPICKUPAMMOVALUE);

	data[static_cast<int>(Pickup::Type::FireRate)].texture = Textures::Entities;
	data[static_cast<int>(Pickup::Type::FireRate)].textureRect = sf::IntRect(120, 64, 40, 40);
	data[static_cast<int>(Pickup::Type::FireRate)].action = std::bind(&Character::increaseFireRate, _1);

	return data;
}


std::vector<ParticleData> initializeParticleData()
{
	std::vector<ParticleData> data(Particle::ParticleCount);

	data[Particle::Exhaust].color = sf::Color(255, 255, 50);
	data[Particle::Exhaust].lifetime = sf::seconds(0.6f);

	data[Particle::Smoke].color = sf::Color(50, 50, 50);
	data[Particle::Smoke].lifetime = sf::seconds(4.f);

	return data;
}

std::vector<MapTileData> initializeMapTileData()
{
	std::vector<MapTileData> data(static_cast<int>(MapTiler::MapID::TypeCount));

	data[static_cast<int>(MapTiler::MapID::Dessert)].mapFile = "Media/map.txt";
	data[static_cast<int>(MapTiler::MapID::Dessert)].texture = Textures::MapTiles;
	data[static_cast<int>(MapTiler::MapID::Dessert)].tileSize = sf::Vector2u(DESSERT_TILE_WIDTH, DESSERT_TILE_HEIGHT);

	return data;
}

//Mike
std::vector<CharacterData> initializeCharacterData()
{
	std::vector<CharacterData> data(static_cast<int>(Character::Type::TypeCount));
	data[static_cast<int>(Character::Type::Player)].hitpoints = PLAYERHITPOINTS;
	data[static_cast<int>(Character::Type::Player)].speed = PLAYERSPEED;
	data[static_cast<int>(Character::Type::Player)].rotationSpeed = ROTATION_SPEED; // Alex - Add rotation speed
	data[static_cast<int>(Character::Type::Player)].fireInterval = sf::seconds(PLAYERFIREINTERVAL);
	data[static_cast<int>(Character::Type::Player)].texture = Textures::ID::PlayerMove;
	data[static_cast<int>(Character::Type::Player)].textureRect = sf::IntRect(
		0, 0, PLAYER_MOVE_ANIMATION_WIDTH, PLAYER_MOVE_ANIMATION_HEIGHT);
	data[static_cast<int>(Character::Type::Player)].moveScale = 0.5f;
	data[static_cast<int>(Character::Type::Player)].deathScale = 0.5f;
	data[static_cast<int>(Character::Type::Player)].moveFrames = 18;
	data[static_cast<int>(Character::Type::Player)].deathFrames = 6;
	data[static_cast<int>(Character::Type::Player)].moveAnimation = Textures::ID::PlayerMove;
	data[static_cast<int>(Character::Type::Player)].moveRect = sf::IntRect(
		0, 0, PLAYER_MOVE_ANIMATION_WIDTH, PLAYER_MOVE_ANIMATION_HEIGHT);
	data[static_cast<int>(Character::Type::Player)].deathAnimation = Textures::ID::PlayerDeath;
	data[static_cast<int>(Character::Type::Player)].deathRect = sf::IntRect(
		0, 0, PLAYER_DEATH_ANIMATION_WIDTH, PLAYER_DEATH_ANIMATION_HEIGHT);

	data[static_cast<int>(Character::Type::Zombie)].hitpoints = ZOMBIEHITPOINTS;
	data[static_cast<int>(Character::Type::Zombie)].speed = ZOMBIESPEED;
	data[static_cast<int>(Character::Type::Zombie)].rotationSpeed = ROTATION_SPEED; // Alex - Add rotation speed
	data[static_cast<int>(Character::Type::Zombie)].texture = Textures::ID::Entities;
	data[static_cast<int>(Character::Type::Zombie)].textureRect = sf::IntRect(
		144, 0, ZOMBIE_MOVE_ANIMATION_WIDTH, ZOMBIE_MOVE_ANIMATION_HEIGHT);
	data[static_cast<int>(Character::Type::Zombie)].fireInterval = sf::Time::Zero;
	data[static_cast<int>(Character::Type::Zombie)].directions.push_back(Direction(+45.f, 80.f));
	data[static_cast<int>(Character::Type::Zombie)].directions.push_back(Direction(-45.f, 160.f));
	data[static_cast<int>(Character::Type::Zombie)].directions.push_back(Direction(+45.f, 80.f));

	data[static_cast<int>(Character::Type::Zombie)].moveAnimation = Textures::ID::ZombieMove;
	data[static_cast<int>(Character::Type::Zombie)].moveRect = sf::IntRect(
		0, 0, ZOMBIE_MOVE_ANIMATION_WIDTH, ZOMBIE_MOVE_ANIMATION_HEIGHT);

	data[static_cast<int>(Character::Type::Zombie)].deathAnimation = Textures::ID::ZombieDeath;
	data[static_cast<int>(Character::Type::Zombie)].deathRect = sf::IntRect(
		0, 0, ZOMBIE_DEATH_ANIMATION_WIDTH, ZOMBIE_DEATH_ANIMATION_HEIGHT);

	data[static_cast<int>(Character::Type::Zombie)].moveScale = 2.f;
	data[static_cast<int>(Character::Type::Zombie)].deathScale = 2.f;
	data[static_cast<int>(Character::Type::Zombie)].moveFrames = 27;
	data[static_cast<int>(Character::Type::Zombie)].deathFrames = 17;

	return data;
}

//Mike
std::vector<ExplosionData> initializeExplosionData()
{
	std::vector<ExplosionData> data(static_cast<int>(Explosion::ExplosionIDs::TypeCount));

	data[static_cast<int>(Explosion::ExplosionIDs::GrenadeExplosion)].damage = GRENADEDAMAGE;
	data[static_cast<int>(Explosion::ExplosionIDs::GrenadeExplosion)].radius = 80;
	data[static_cast<int>(Explosion::ExplosionIDs::GrenadeExplosion)].lifeTimeSeconds = 1;
	data[static_cast<int>(Explosion::ExplosionIDs::GrenadeExplosion)].texture = Textures::ID::Entities;
	data[static_cast<int>(Explosion::ExplosionIDs::GrenadeExplosion)].textureRect = sf::IntRect(0, 0, 150, 150);

	return data;
}

//Mike


//Mike
std::vector<ObstacleData> initializeObstacleData()
{
	return std::vector<ObstacleData>();
}

