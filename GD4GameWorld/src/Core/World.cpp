#include "World.hpp"
#include "Projectile.hpp"
#include "Pickup.hpp"
#include "ParticleNode.hpp"
#include "PostEffect.hpp"
#include "SoundNode.hpp"
#include "Constants.hpp"
#include "MapTiler.hpp"
#include "Obstacle.hpp"
#include "Debugger.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds)
	: mTarget(outputTarget)
	  , mWorldView(outputTarget.getDefaultView())
	  , mFonts(fonts)
	  , mSounds(sounds)
	  , mSceneLayers()
	  , mSpawnPosition(mWorldView.getCenter())
	  , mScrollSpeed(0.f)
	  , mPlayerCharacter(nullptr)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	// Alex - Apply zoom factor
	mWorldView.zoom(LEVEL_ZOOM_FACTOR);

	loadTextures();
	buildScene();
}

void World::loadTextures()
{
	//TODO PLACE SEPARATE TEXTURES INTO SPRITE SHEETS, REMOVE unused ASSETS
	mTextures.load(TextureIDs::Entities, "Media/Textures/Entities.png");
	mTextures.load(TextureIDs::Water, "Media/Textures/Water.jpg");
	mTextures.load(TextureIDs::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(TextureIDs::Particle, "Media/Textures/Particle.png");

	mTextures.load(TextureIDs::PlayerMove, "Media/Textures/PlayerMove.png");
	mTextures.load(TextureIDs::PlayerDeath, "Media/Textures/Blood.png");

	mTextures.load(TextureIDs::ZombieMove, "Media/Textures/ZombieWalk.png");
	mTextures.load(TextureIDs::ZombieDeath, "Media/Textures/ZombieDeath.png");

	mTextures.load(TextureIDs::Grenade, "Media/Textures/Grenade.png");
	mTextures.load(TextureIDs::MapTiles, "Media/Textures/Tiles.png");

	mTextures.load(TextureIDs::Crate, "Media/Textures/Crate.png");
}

#pragma region Getters

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}

sf::FloatRect World::getViewBounds() const
{
	return sf::FloatRect(mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize());
}

sf::FloatRect World::getBattlefieldBounds() const
{
	sf::FloatRect bounds = getViewBounds();
	return bounds;
}
#pragma endregion

#pragma region Update

void World::update(sf::Time dt)
{
#pragma region Author: Alex

	// Scroll the world, reset player velocity
	//mWorldView.move(0.f, mScrollSpeed * dt.asSeconds());

	//sf::Vector2f playerVelocity = mPlayerCharacter->getVelocity();

	// Stick view to player position
	mWorldView.setCenter(mPlayerCharacter->getPosition());
	/*
	Quick Alternative To:
	mWorldView.move(playerVelocity.x * dt.asSeconds(), playerVelocity.y * dt.asSeconds());
	*/

	// Alex - Handle player collisions (e.g. prevent leaving battlefield)
	handlePlayerCollision();

#pragma endregion

	mPlayerCharacter->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide grenades
	destroyEntitiesOutsideView();
	guideZombies();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);
	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();


	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();
	updateSounds();

	spawnEnemies();

	//Debugger::GetInstance().LogVector("Player Position", mPlayerCharacter->getPosition(), 0);
	//Debugger::GetInstance().LogVector("Player Velocity", mPlayerCharacter->getVelocity(), 1);
	//Debugger::GetInstance().LogInt("Player Hitpoints: ", mPlayerCharacter->getHitpoints(), 2);
}

void World::draw()
{
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear(sf::Color(BLUE));
		mSceneTexture.setView(mWorldView);
		mSceneTexture.draw(mSceneGraph);
		mSceneTexture.display();
		mBloomEffect.apply(mSceneTexture, mTarget);
	}
	else
	{
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	sf::Vector2f position = mPlayerCharacter->getPosition();
	position.x = std::max(position.x, viewBounds.left + borderDistance);
	position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
	position.y = std::max(position.y, viewBounds.top + borderDistance);
	position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
	mPlayerCharacter->setPosition(position);
}

void World::adaptPlayerVelocity()
{
	sf::Vector2f velocity = mPlayerCharacter->getVelocity();

	// If moving diagonally, reduce velocity (to have always same velocity)
	if (velocity.x != 0.f && velocity.y != 0.f)
		mPlayerCharacter->setVelocity(velocity / std::sqrt(2.f));

	// Add scrolling velocity
	mPlayerCharacter->accelerate(0.f, mScrollSpeed);
}

bool World::hasAlivePlayer() const
{
	return !mPlayerCharacter->isMarkedForRemoval();
}

bool World::hasPlayerReachedEnd() const
{
	return !mWorldBounds.contains(mPlayerCharacter->getPosition());
}

void World::updateSounds()
{
	// Set listener's position to player position
	mSounds.setListenerPosition(mPlayerCharacter->getWorldPosition());

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

void World::spawnEnemies()
{
	// Spawn all enemies entering the view area (including distance) this frame
	while (!mEnemySpawnPoints.empty()
		&& mEnemySpawnPoints.back().y > getBattlefieldBounds().top)
	{
		SpawnPoint spawn = mEnemySpawnPoints.back();

		std::unique_ptr<Character> enemy(new Character(spawn.type, mTextures, mFonts));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(180.f);

		mSceneLayers[UpperLayer]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();
	}
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(Category::Projectile) | static_cast<int>(Category::EnemyCharacter);
	command.action = derivedAction<Entity>([this](Entity& e, sf::Time)
	{
		if (!getBattlefieldBounds().intersects(e.getBoundingRect()))
			e.destroy();
	});

	mCommandQueue.push(command);
}

void World::guideZombies()
{
	// Setup command that stores all players in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = static_cast<int>(Category::PlayerCharacter);
	enemyCollector.action = derivedAction<Character>([this](Character& player, sf::Time)
	{
		if (!player.isDestroyed())
			mActiveEnemies.push_back(&player);
	});

	// Setup command that guides all grenades to the enemy which is currently closest to the player
	Command zombieGuider;
	zombieGuider.category = static_cast<int>(Category::EnemyCharacter);
	zombieGuider.action = derivedAction<Character>([this](Character& zombie, sf::Time)
	{
		// Ignore unguided players
		if (zombie.isPlayer())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Character* closestEnemy = nullptr;

		// Find closest enemy
		for (Character* enemy : mActiveEnemies)
		{
			float enemyDistance = distance(zombie, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			zombie.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(zombieGuider);
	mActiveEnemies.clear();
}

#pragma endregion

#pragma region Creation

void World::buildScene()
{


	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		Category category = i == LowerLayer ? Category::SceneLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	std::unique_ptr<MapTiler> map(new MapTiler(MapTiler::MapID::Dessert, mTextures));
	mWorldBounds = map->getMapBounds();
	sf::Texture& waterTexture = mTextures.get(TextureIDs::Water);
	waterTexture.setRepeated(true);

	float viewHeight = mWorldBounds.height;
	float viewWidth = mWorldBounds.width;

	sf::IntRect textureRect(sf::FloatRect(128, 128, mWorldBounds.width * 2, mWorldBounds.height * 2));
	textureRect.height += static_cast<int>(viewHeight);

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> waterSprite(new SpriteNode(waterTexture, textureRect));
	waterSprite->setPosition(-viewWidth / 2, -viewHeight);
	mSceneLayers[Layer::Background]->attachChild(std::move(waterSprite));

	map->setPosition(mWorldBounds.left, mWorldBounds.top);

	mSceneLayers[Background]->attachChild(std::move(map));

	// Add particle node to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(Particle::Type::Smoke, mTextures));
	mSceneLayers[LowerLayer]->attachChild(std::move(smokeNode));

	// Add exhaust particle node to the scene
	std::unique_ptr<ParticleNode> exhaustNode(new ParticleNode(Particle::Type::Exhaust, mTextures));
	mSceneLayers[LowerLayer]->attachChild(std::move(exhaustNode));

	//Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
	mSceneGraph.attachChild(std::move(soundNode));

	// Add player's Character
	std::unique_ptr<Character> player(new Character(Character::Type::Player, mTextures, mFonts));
	mPlayerCharacter = player.get();
	mPlayerCharacter->setPosition(mSpawnPosition);
	mSceneLayers[UpperLayer]->attachChild(std::move(player));

	//std::unique_ptr<Obstacle> obstacle(new Obstacle(Obstacle::ObstacleID::Crate, mTextures));
	//obstacle->setPosition(sf::Vector2f(1000, 500));
	//mSceneLayers[UpperAir]->attachChild(std::move(obstacle));

	//std::unique_ptr<Obstacle> obstacle(new Obstacle(Obstacle::ObstacleID::Crate, mTextures, 50));
	//obstacle->setPosition(sf::Vector2f(200, 50));
	//mSceneLayers[UpperAir]->attachChild(std::move(obstacle));

	createObstacle(mSceneGraph, mTextures, sf::Vector2f(250, 250));

	createObstacle(mSceneGraph, mTextures, sf::Vector2f(450, 250));

	createObstacle(mSceneGraph, mTextures, sf::Vector2f(650, 250));

	createObstacle(mSceneGraph, mTextures, sf::Vector2f(850, 250));

	// Add enemy Character
	addEnemies();
}

void World::addEnemies()
{
	// Add enemies to the spawn point container

	addEnemy(Character::Type::Zombie, -1000.f, -1000.f);
	addEnemy(Character::Type::Zombie, 1000.f, 500.f);
	addEnemy(Character::Type::Zombie, 4000.f, 1150.f);
	addEnemy(Character::Type::Zombie, 8000.f, -1220.f);
	addEnemy(Character::Type::Zombie, -3000.f, -1220.f);
	addEnemy(Character::Type::Zombie, 1100.f, 20.f);
	addEnemy(Character::Type::Zombie, 2000.f, -500.f);
	addEnemy(Character::Type::Zombie, 6000.f, -300.f);
	addEnemy(Character::Type::Zombie, -250.f, -3020.f);
	addEnemy(Character::Type::Zombie, -250.f, 3250.f);
	addEnemy(Character::Type::Zombie, 600.f, 800.f);
	addEnemy(Character::Type::Zombie, 800.f, 400.f);
	addEnemy(Character::Type::Zombie, 450.f, 200.f);
	// Sort all enemies according to their y value, such that lower enemies are checked first for spawning
	std::sort(mEnemySpawnPoints.begin(), mEnemySpawnPoints.end(), [](SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.y < rhs.y;
	});
}

void World::addEnemy(Character::Type type, float relX, float relY)
{
	SpawnPoint spawn(type, mSpawnPosition.x + relX, mSpawnPosition.y - relY);
	mEnemySpawnPoints.push_back(spawn);
}

void World::createObstacle(SceneNode& node, const TextureHolder& textures, sf::Vector2f position) const
{
	std::unique_ptr<Obstacle> obstacle(new Obstacle(Obstacle::ObstacleID::Crate, textures));
	obstacle->setPosition(position);
	node.attachChild(std::move(obstacle));
}

#pragma endregion

#pragma region Collisions
bool matchesCategories(SceneNode::Pair& colliders, Category type1, Category type2)
{
	unsigned int category1 = colliders.first->getCategory();
	unsigned int category2 = colliders.second->getCategory();

	// Make sure first pair entry has category type1 and second has type2
	if (static_cast<int>(type1) & category1 && static_cast<int>(type2) & category2)
	{
		return true;
	}
	if (static_cast<int>(type1) & category2 && static_cast<int>(type2) & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	return false;
}

void World::handlePlayerCollision()
{
	//TODO make map member variable and change 128 to gettileSize()
	if (!shrink(128, mWorldBounds).contains(mPlayerCharacter->getPosition()))
	{
		mPlayerCharacter->setPosition(mPlayerCharacter->getLastPosition());
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	for (SceneNode::Pair pair : collisionPairs)
	{
		CollisionType collisionType = GetCollisionType(pair);
		switch (collisionType)
		{
		case Character_Character:
			handleCharacterCollisions(pair);
			break;
		case Player_Pickup:
			handlePickupCollisions(pair);
			break;
		case Player_Obstacle:
			handleObstacleCollisions(pair);
			break;
		case Projectile_Obstacle:
		case Projectile_Character:
			handleProjectileCollisions(pair);
			break;
		case Character_Explosion:
			handleExplosionCollisions(pair);
			break;
		default:
			break;
		}
	}
}

void World::handleCharacterCollisions(SceneNode::Pair& pair)
{
	auto& character1 = static_cast<Character&>(*pair.first);
	auto& character2 = static_cast<Character&>(*pair.second);

	if ((character1.isZombie() && character2.isZombie())
		|| (character1.isPlayer() && character2.isPlayer()))
	{
		//TODO FIX COLLISION
		character1.setPosition(character1.getLastPosition());
		character2.setPosition(character2.getLastPosition());
	}
	else
	{
		character1.damage(1);
	}
}

void World::handleObstacleCollisions(SceneNode::Pair& pair)
{
	auto& character = static_cast<Character&>(*pair.first);
	auto& obstacle = static_cast<Obstacle&>(*pair.second);
	character.setPosition(character.getLastPosition());
}

void World::handleProjectileCollisions(SceneNode::Pair& pair)
{
	auto& entity = static_cast<Entity&>(*pair.first);
	auto& projectile = static_cast<Projectile&>(*pair.second);

	if (projectile.isGrenade())
	{
		projectile.setVelocity(-projectile.getVelocity() / 2.f);
	}
	else
	{
		entity.damage(projectile.getDamage());
		projectile.destroy();
	}
}

void World::handlePickupCollisions(SceneNode::Pair& pair)
{
	auto& player = static_cast<Character&>(*pair.first);
	auto& pickup = static_cast<Pickup&>(*pair.second);

	// Apply pickup effect to player, destroy projectile
	//TODO APPLY PICKUP EFFECTS
	pickup.apply(player);
	pickup.destroy();

	player.playLocalSound(mCommandQueue, SoundEffectIDs::CollectPickup);
}

void World::handleExplosionCollisions(SceneNode::Pair& pair)
{
	auto& entity = static_cast<Entity&>(*pair.first);
	auto& explosion = static_cast<Explosion&>(*pair.second);

	// Distance between characters and explosions center-points
	//float dV = vectorDistance(entity.getWorldPosition(), explosion.getWorldPosition());

	entity.damage(1);
}

World::CollisionType World::GetCollisionType(SceneNode::Pair& pair)
{
	if (matchesCategories(pair, Category::PlayerCharacter, Category::EnemyCharacter)
		|| matchesCategories(pair, Category::EnemyCharacter, Category::EnemyCharacter))
	{
		return Character_Character;
	}
	else if (matchesCategories(pair, Category::PlayerCharacter, Category::Pickup))
	{
		return Player_Pickup;
	}
	else if (matchesCategories(pair, Category::PlayerCharacter, Category::Obstacle))
	{
		return Player_Obstacle;
	}
	else if (matchesCategories(pair, Category::Obstacle, Category::AlliedProjectile)
		|| matchesCategories(pair, Category::Obstacle, Category::EnemyProjectile))
	{
		return Projectile_Obstacle;
	}
	else if (matchesCategories(pair, Category::EnemyCharacter, Category::AlliedProjectile)
		|| matchesCategories(pair, Category::PlayerCharacter, Category::EnemyProjectile))
	{
		return Projectile_Character;
	}
	else if (matchesCategories(pair, Category::EnemyCharacter, Category::Explosion)
		|| matchesCategories(pair, Category::PlayerCharacter, Category::Explosion))
	{
		return Character_Explosion;
	}
	else { return Default; }
}
#pragma endregion
