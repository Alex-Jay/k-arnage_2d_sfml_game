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
	  // Alex - Increased map size
	  , mWorldBounds(0.f, 0.f, mWorldView.getSize().x * 2, 1000.f)
	  , mSpawnPosition(mWorldView.getSize().x / 2.f, mWorldBounds.height - mWorldView.getSize().y / 2.f)
	  , mScrollSpeed(0.f)
	  , mPlayerCharacter(nullptr)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	// Alex - Apply zoom factor
	mWorldView.zoom(LEVEL_ZOOM_FACTOR);

	loadTextures();
	buildScene();

	// Prepare the view
	//mWorldView.setCenter(mSpawnPosition);
}

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
	//MapTiler m;
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear(sf::Color(114, 168, 255, 255));
		mSceneTexture.setView(mWorldView);
		mSceneTexture.draw(mSceneGraph);
		mSceneTexture.display();
		mBloomEffect.apply(mSceneTexture, mTarget);
		//mTarget.draw(m);
	}
	else
	{
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
		//mTarget.draw(m);
	}
}

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}

bool World::hasAlivePlayer() const
{
	return !mPlayerCharacter->isMarkedForRemoval();
}

bool World::hasPlayerReachedEnd() const
{
	return !mWorldBounds.contains(mPlayerCharacter->getPosition());
}

void World::loadTextures()
{

	//TODO PLACE SEPARATE TEXTURES INTO SPRITE SHEETS
	mTextures.load(TextureIDs::Entities, "Media/Textures/Entities.png");
	mTextures.load(TextureIDs::Sand, "Media/Textures/Sand.png");
	mTextures.load(TextureIDs::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(TextureIDs::Particle, "Media/Textures/Particle.png");
	mTextures.load(TextureIDs::FinishLine, "Media/Textures/FinishLine.png");

	mTextures.load(TextureIDs::PlayerMove, "Media/Textures/PlayerMove.png");
	mTextures.load(TextureIDs::PlayerDeath, "Media/Textures/Blood.png");

	mTextures.load(TextureIDs::ZombieMove, "Media/Textures/ZombieWalk.png");
	mTextures.load(TextureIDs::ZombieDeath, "Media/Textures/ZombieDeath.png");

	mTextures.load(TextureIDs::Grenade, "Media/Textures/Grenade.png");
	mTextures.load(TextureIDs::MapTiles, "Media/Textures/Tiles.png");

	mTextures.load(TextureIDs::Crate, "Media/Textures/Crate.png");
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
	// Check if player is out of bounds (Left side)
	if (mPlayerCharacter->getPosition().x <= 0.0f)
	{
		mPlayerCharacter->setPosition(0.0f, mPlayerCharacter->getPosition().y);
	}
	// Check if player is out of bounds (right side)
	if (mPlayerCharacter->getPosition().x >= mWorldBounds.width)
	{
		mPlayerCharacter->setPosition(mWorldBounds.width, mPlayerCharacter->getPosition().y);
	}
	// Check if player is out of bounds (top side)
	if (mPlayerCharacter->getPosition().y <= 0.0f)
	{
		mPlayerCharacter->setPosition(mPlayerCharacter->getPosition().x, 0.0f);
	}
	/*
		Check if player is out of bounds (bottom side)
		getBattlefieldBounds().height + mWorldBounds.height = Height of battlefield added on with World bound height
	*/
	if (mPlayerCharacter->getPosition().y >= getBattlefieldBounds().height + mWorldBounds.height)
	{
		mPlayerCharacter->setPosition(mPlayerCharacter->getPosition().x,
		                              getBattlefieldBounds().height + mWorldBounds.height);
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);
	//TODO Replace this stuff wit a switch statement and make it more readable/efficient
	//Have one check for collisons e.g
	// if collidee is an obstacle then handle that by checking if the collider is a player, bullet, etc..
	for (SceneNode::Pair pair : collisionPairs)
	{
		if (matchesCategories(pair, Category::PlayerCharacter, Category::EnemyCharacter))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& enemy = static_cast<Character&>(*pair.second);

			// Alex - Disabled player to enemy damage
			// Collision: Player damage = enemy's remaining HP
			//player.damage(enemy.getHitpoints());
			//enemy.destroy();
		}
		else if (matchesCategories(pair, Category::PlayerCharacter, Category::Pickup))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			//pickup.apply(player);
			pickup.destroy();

			//TODO Change that annoying ass pickup sound
			player.playLocalSound(mCommandQueue, SoundEffectIDs::CollectPickup);
		}
		else if (matchesCategories(pair, Category::PlayerCharacter, Category::Obstacle))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& obstacle = static_cast<Obstacle&>(*pair.second);
			//auto& pickup = static_cast<Obstacle&>(*pair.second);


			//Debugger::GetInstance().LogVector("Player Collision", player.getPosition(), 0);
			//Debugger::GetInstance().LogVector("Obstacle", obstacle.getPosition(), 1);

			// Apply pickup effect to player, destroy projectile
			//pickup.apply(player);
			//pickup.destroy();
			//TODO handle collisions better
			//player.setVelocity(-player.getVelocity());
		}
		else if (matchesCategories(pair, Category::Obstacle, Category::AlliedProjectile)
			|| matchesCategories(pair, Category::Obstacle, Category::EnemyProjectile))
		{
			auto& projectile = static_cast<Projectile&>(*pair.second);
			auto& obstacle = static_cast<Obstacle&>(*pair.first);

			// Apply pickup effect to player, destroy projectile
			//pickup.apply(player);
			projectile.destroy();
			//DONE Fixed bug, Entity is removed automatically if its hitpoints drops below 0
			if (!obstacle.isDestroyed())
			{
				obstacle.damage(10);
				//obstacle.setVelocity(100, -200);
			}
			else
			{
				obstacle.checkPickupDrop(mCommandQueue);
			}
		}
		else if (matchesCategories(pair, Category::EnemyCharacter, Category::AlliedProjectile)
			|| matchesCategories(pair, Category::PlayerCharacter, Category::EnemyProjectile))
		{
			auto& character = static_cast<Character&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// If is projectile
			if (!projectile.isGrenade())
			{
				// Apply projectile damage to Character, destroy projectile
				character.damage(projectile.getDamage());
				projectile.destroy();
			}

			if (projectile.isGrenade())
			{
				// Stop grenade on impact
				projectile.setVelocity(0, 0);
			}
		}
		else if (matchesCategories(pair, Category::EnemyCharacter, Category::Explosion)
			|| matchesCategories(pair, Category::PlayerCharacter, Category::Explosion))
		{
			auto& character = static_cast<Character&>(*pair.first);
			auto& explosion = static_cast<Explosion&>(*pair.second);

			// Distance between characters and explosions center-points
			float dV = vectorDistance(character.getWorldPosition(), explosion.getWorldPosition());

			// Can't simply use character.damage() here upon explosion
			// This is a for-loop, meaning that .damage() will be called as many times as the player or zombie is inside the collision rectangle
			// Eventually, resulting in Access Violation error since the object is deleted but is still trying to damage it
		}
	}
}

void World::updateSounds()
{
	// Set listener's position to player position
	mSounds.setListenerPosition(mPlayerCharacter->getWorldPosition());

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

void World::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		Category category = i == LowerAir ? Category::SceneAirLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	// Prepare the tiled background
	std::unique_ptr<MapTiler> map(new MapTiler(MapTiler::MapID::Dessert, mTextures));

	map->setPosition(mWorldBounds.left, mWorldBounds.top);

	mSceneLayers[Background]->attachChild(std::move(map));

	// Add particle node to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(Particle::Type::Smoke, mTextures));
	mSceneLayers[LowerAir]->attachChild(std::move(smokeNode));

	// Add exhaust particle node to the scene
	std::unique_ptr<ParticleNode> exhaustNode(new ParticleNode(Particle::Type::Exhaust, mTextures));
	mSceneLayers[LowerAir]->attachChild(std::move(exhaustNode));

	//Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
	mSceneGraph.attachChild(std::move(soundNode));

	// Add player's Character
	std::unique_ptr<Character> player(new Character(Character::Type::Player, mTextures, mFonts));
	mPlayerCharacter = player.get();
	mPlayerCharacter->setPosition(mSpawnPosition);
	mSceneLayers[UpperAir]->attachChild(std::move(player));

	//std::unique_ptr<Obstacle> obstacle(new Obstacle(Obstacle::ObstacleID::Crate, mTextures));
	//obstacle->setPosition(sf::Vector2f(1000, 500));
	//mSceneLayers[UpperAir]->attachChild(std::move(obstacle));
	
	//std::unique_ptr<Obstacle> obstacle(new Obstacle(Obstacle::ObstacleID::Crate, mTextures, 50));
	//obstacle->setPosition(sf::Vector2f(200, 50));
	//mSceneLayers[UpperAir]->attachChild(std::move(obstacle));
	createObstacle(mSceneGraph, mTextures, sf::Vector2f(50, 50), 50);

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

		mSceneLayers[UpperAir]->attachChild(std::move(enemy));

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
		// Ignore unguided bullets
		if (zombie.isAllied())
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

sf::FloatRect World::getViewBounds() const
{
	return sf::FloatRect(mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize());
}

sf::FloatRect World::getBattlefieldBounds() const
{
	// Return view bounds + some area at top, where enemies spawn
	sf::FloatRect bounds = getViewBounds();
	// Alex - Disable extra bounding area
	//bounds.top -= 100.f;
	//bounds.height += 100.f;

	return bounds;
}

void World::createObstacle(SceneNode& node, const TextureHolder& textures, sf::Vector2f position, int obstacleHitpoints) const
{
	std::unique_ptr<Obstacle> obstacle(new Obstacle(Obstacle::ObstacleID::Crate, textures, obstacleHitpoints));
	obstacle->setPosition(position);
	node.attachChild(std::move(obstacle));
}