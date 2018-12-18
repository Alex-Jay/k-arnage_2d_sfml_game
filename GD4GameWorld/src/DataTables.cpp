#include "DataTables.hpp"
#include "Character.hpp"
#include "Character.hpp"
#include "Projectile.hpp"
#include "Particle.hpp"
#include "Pickup.hpp"
#include "Constants.hpp"

using namespace std::placeholders;

std::vector<CharacterData> initializeCharacterData()
{
	std::vector<CharacterData> data(static_cast<int>(Character::Type::TypeCount));
	data[static_cast<int>(Character::Type::Eagle)].hitpoints = EAGLEHITPOINTS;
	data[static_cast<int>(Character::Type::Eagle)].speed = 200.f;	
	data[static_cast<int>(Character::Type::Eagle)].fireInterval = sf::seconds(1);
	data[static_cast<int>(Character::Type::Eagle)].texture = TextureIDs::PlayerMove;
	data[static_cast<int>(Character::Type::Eagle)].textureRect = sf::IntRect(0, 0, PLAYERANIMATIONRECTWIDTH, PLAYERANIMATIONRECTHEIGHT);
	data[static_cast<int>(Character::Type::Eagle)].hasRollAnimation = true;

	data[static_cast<int>(Character::Type::Raptor)].hitpoints = 20;
	data[static_cast<int>(Character::Type::Raptor)].speed = 80.f;
	data[static_cast<int>(Character::Type::Raptor)].texture = TextureIDs::Entities;
	data[static_cast<int>(Character::Type::Raptor)].textureRect = sf::IntRect(144, 0, 84, 64);
	data[static_cast<int>(Character::Type::Raptor)].fireInterval = sf::Time::Zero;
	data[static_cast<int>(Character::Type::Raptor)].directions.push_back(Direction(+45.f, 80.f));
	data[static_cast<int>(Character::Type::Raptor)].directions.push_back(Direction(-45.f, 160.f));
	data[static_cast<int>(Character::Type::Raptor)].directions.push_back(Direction(+45.f, 80.f));
	data[static_cast<int>(Character::Type::Raptor)].hasRollAnimation = false;

	data[static_cast<int>(Character::Type::Avenger)].hitpoints = 40;
	data[static_cast<int>(Character::Type::Avenger)].speed = 50.f;
	data[static_cast<int>(Character::Type::Avenger)].texture = TextureIDs::Entities;
	data[static_cast<int>(Character::Type::Avenger)].textureRect = sf::IntRect(228, 0, 60, 59);
	data[static_cast<int>(Character::Type::Avenger)].fireInterval = sf::seconds(2);
	data[static_cast<int>(Character::Type::Avenger)].directions.push_back(Direction(+45.f, 50.f));					
	data[static_cast<int>(Character::Type::Avenger)].directions.push_back(Direction(0.f, 50.f));				
	data[static_cast<int>(Character::Type::Avenger)].directions.push_back(Direction(-45.f, 100.f));
	data[static_cast<int>(Character::Type::Avenger)].directions.push_back(Direction(0.f, 50.f));
	data[static_cast<int>(Character::Type::Avenger)].directions.push_back(Direction(+45.f, 50.f));
	data[static_cast<int>(Character::Type::Avenger)].hasRollAnimation = false;

	return data;
}

//std::vector<CharacterData> initializeCharacterData()
//{
//	std::vector<CharacterData> data(static_cast<int>(Character::Type::TypeCount));
//	data[static_cast<int>(Character::Type::Player)].hitpoints = PLAYERHITPOINTS;
//	data[static_cast<int>(Character::Type::Player)].speed = CHARACTERSPEED;
//	data[static_cast<int>(Character::Type::Player)].fireInterval = sf::seconds(PLAYERFIREINTERVAL);
//	data[static_cast<int>(Character::Type::Player)].texture = TextureIDs::PlayerMove;
//	data[static_cast<int>(Character::Type::Player)].textureRect = sf::IntRect(0, 0, PLAYERANIMATIONRECTWIDTH, PLAYERANIMATIONRECTHEIGHT);
//	data[static_cast<int>(Character::Type::Player)].hasAnimation = true;
//
//	//data[static_cast<int>(Character::Type::Zombie)].hitpoints = PLAYERHITPOINTS;
//	//data[static_cast<int>(Character::Type::Zombie)].speed = CHARACTERSPEED;
//	//data[static_cast<int>(Character::Type::Zombie)].fireInterval = sf::seconds(PLAYERFIREINTERVAL);
//	//data[static_cast<int>(Character::Type::Zombie)].texture = TextureIDs::PlayerMove;
//	//data[static_cast<int>(Character::Type::Zombie)].textureRect = sf::IntRect(0, 0, PLAYERANIMATIONRECTWIDTH, PLAYERANIMATIONRECTHEIGHT);
//	//data[static_cast<int>(Character::Type::Zombie)].hasAnimation = true;
//
//	return data;
//}
std::vector<ProjectileData> initializeProjectileData()
{
	std::vector<ProjectileData> data(static_cast<int>(Projectile::ProjectileIDs::TypeCount));

	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].damage = 10;
	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].speed = 300.f;
	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].texture = TextureIDs::Entities;
	data[static_cast<int>(Projectile::ProjectileIDs::AlliedBullet)].textureRect = sf::IntRect(175, 64, 3, 14);

	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].damage = 10;
	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].speed = 300.f;
	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].texture = TextureIDs::Entities;
	data[static_cast<int>(Projectile::ProjectileIDs::EnemyBullet)].textureRect = sf::IntRect(178, 64, 3, 14);

	data[static_cast<int>(Projectile::ProjectileIDs::Missile)].damage = 200;
	data[static_cast<int>(Projectile::ProjectileIDs::Missile)].speed = 150.f;
	data[static_cast<int>(Projectile::ProjectileIDs::Missile)].texture = TextureIDs::Entities;
	data[static_cast<int>(Projectile::ProjectileIDs::Missile)].textureRect = sf::IntRect(160, 64, 15, 32);

	return data;
}

std::vector<PickupData> initializePickupData()
{
	std::vector<PickupData> data(static_cast<int>(Pickup::PickupID::TypeCount));

	data[static_cast<int>(Pickup::PickupID::HealthRefill)].texture = TextureIDs::Entities;
	data[static_cast<int>(Pickup::PickupID::HealthRefill)].textureRect = sf::IntRect(0, 64, 40, 40);
	data[static_cast<int>(Pickup::PickupID::HealthRefill)].action = [](Character& a) {
		a.repair(HEALTHPICKUPREPAIRVALUE); };

	data[static_cast<int>(Pickup::PickupID::MissileRefill)].texture = TextureIDs::Entities;
	data[static_cast<int>(Pickup::PickupID::MissileRefill)].textureRect = sf::IntRect(40, 64, 40, 40);
	data[static_cast<int>(Pickup::PickupID::MissileRefill)].action = std::bind(&Character::collectMissiles, _1, GrenadePICKUPAMMOVALUE);

	data[static_cast<int>(Pickup::PickupID::FireSpread)].texture = TextureIDs::Entities;
	data[static_cast<int>(Pickup::PickupID::FireSpread)].textureRect = sf::IntRect(80, 64, 40, 40);
	data[static_cast<int>(Pickup::PickupID::FireSpread)].action = std::bind(&Character::increaseSpread, _1);

	data[static_cast<int>(Pickup::PickupID::FireRate)].texture = TextureIDs::Entities;
	data[static_cast<int>(Pickup::PickupID::FireRate)].textureRect = sf::IntRect(120, 64, 40, 40);
	data[static_cast<int>(Pickup::PickupID::FireRate)].action = std::bind(&Character::increaseFireRate, _1);

	return data;
}

std::vector<ParticleData> initializeParticleData()
{
	std::vector<ParticleData> data(static_cast<int>(Particle::Type::ParticleCount));
	data[static_cast<int>(Particle::Type::Exhaust)].color = sf::Color(255, 255, 50);
	data[static_cast<int>(Particle::Type::Exhaust)].lifetime = sf::seconds(0.6f);

	data[static_cast<int>(Particle::Type::Smoke)].color = sf::Color(50, 50, 50);
	data[static_cast<int>(Particle::Type::Smoke)].lifetime = sf::seconds(4.0);

	return data;
}
