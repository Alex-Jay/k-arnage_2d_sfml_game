#include "DataTables.hpp"
#include "Aircraft.hpp"
#include "Projectile.hpp"
#include "Particle.hpp"
#include "Pickup.hpp"
#include "Constants.hpp"

using namespace std::placeholders;

std::vector<AircraftData> initializeAircraftData()
{
	std::vector<AircraftData> data(static_cast<int>(Aircraft::Type::TypeCount));
	data[static_cast<int>(Aircraft::Type::Eagle)].hitpoints = EAGLEHITPOINTS;
	data[static_cast<int>(Aircraft::Type::Eagle)].speed = 200.f;	
	data[static_cast<int>(Aircraft::Type::Eagle)].fireInterval = sf::seconds(1);
	data[static_cast<int>(Aircraft::Type::Eagle)].texture = TextureIDs::Entities;
	data[static_cast<int>(Aircraft::Type::Eagle)].textureRect = sf::IntRect(0,0, 48, 64);
	data[static_cast<int>(Aircraft::Type::Eagle)].hasRollAnimation = true;

	data[static_cast<int>(Aircraft::Type::Raptor)].hitpoints = 20;
	data[static_cast<int>(Aircraft::Type::Raptor)].speed = 80.f;
	data[static_cast<int>(Aircraft::Type::Raptor)].texture = TextureIDs::Entities;
	data[static_cast<int>(Aircraft::Type::Raptor)].textureRect = sf::IntRect(144, 0, 84, 64);
	data[static_cast<int>(Aircraft::Type::Raptor)].fireInterval = sf::Time::Zero;
	data[static_cast<int>(Aircraft::Type::Raptor)].directions.push_back(Direction(+45.f, 80.f));
	data[static_cast<int>(Aircraft::Type::Raptor)].directions.push_back(Direction(-45.f, 160.f));
	data[static_cast<int>(Aircraft::Type::Raptor)].directions.push_back(Direction(+45.f, 80.f));
	data[static_cast<int>(Aircraft::Type::Raptor)].hasRollAnimation = false;

	data[static_cast<int>(Aircraft::Type::Avenger)].hitpoints = 40;
	data[static_cast<int>(Aircraft::Type::Avenger)].speed = 50.f;
	data[static_cast<int>(Aircraft::Type::Avenger)].texture = TextureIDs::Entities;
	data[static_cast<int>(Aircraft::Type::Avenger)].textureRect = sf::IntRect(228, 0, 60, 59);
	data[static_cast<int>(Aircraft::Type::Avenger)].fireInterval = sf::seconds(2);
	data[static_cast<int>(Aircraft::Type::Avenger)].directions.push_back(Direction(+45.f, 50.f));					
	data[static_cast<int>(Aircraft::Type::Avenger)].directions.push_back(Direction(0.f, 50.f));				
	data[static_cast<int>(Aircraft::Type::Avenger)].directions.push_back(Direction(-45.f, 100.f));
	data[static_cast<int>(Aircraft::Type::Avenger)].directions.push_back(Direction(0.f, 50.f));
	data[static_cast<int>(Aircraft::Type::Avenger)].directions.push_back(Direction(+45.f, 50.f));
	data[static_cast<int>(Aircraft::Type::Avenger)].hasRollAnimation = false;

	return data;
}

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
	data[static_cast<int>(Pickup::PickupID::HealthRefill)].action = [](Aircraft& a) {
		a.repair(HEALTHPICKUPREPAIRVALUE); };

	data[static_cast<int>(Pickup::PickupID::MissileRefill)].texture = TextureIDs::Entities;
	data[static_cast<int>(Pickup::PickupID::MissileRefill)].textureRect = sf::IntRect(40, 64, 40, 40);
	data[static_cast<int>(Pickup::PickupID::MissileRefill)].action = std::bind(&Aircraft::collectMissiles, _1, MISSILEPICKUPAMMOVALUE);

	data[static_cast<int>(Pickup::PickupID::FireSpread)].texture = TextureIDs::Entities;
	data[static_cast<int>(Pickup::PickupID::FireSpread)].textureRect = sf::IntRect(80, 64, 40, 40);
	data[static_cast<int>(Pickup::PickupID::FireSpread)].action = std::bind(&Aircraft::increaseSpread, _1);

	data[static_cast<int>(Pickup::PickupID::FireRate)].texture = TextureIDs::Entities;
	data[static_cast<int>(Pickup::PickupID::FireRate)].textureRect = sf::IntRect(120, 64, 40, 40);
	data[static_cast<int>(Pickup::PickupID::FireRate)].action = std::bind(&Aircraft::increaseFireRate, _1);

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
