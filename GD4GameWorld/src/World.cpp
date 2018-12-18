#include "World.hpp"
#include "Projectile.hpp"
#include "Pickup.hpp"
#include "TextNode.hpp"
#include "ParticleNode.hpp"
#include "PostEffect.hpp"
#include "SoundNode.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>


World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds)
	: mTarget(outputTarget)
	, mSceneTexture()
	, mWorldView(outputTarget.getDefaultView())
	, mTextures()
	, mFonts(fonts)
	, mSounds(sounds)
	, mSceneGraph()
	, mSceneLayers()
	// Decreased map size
	, mWorldBounds(0.f, 0.f, mWorldView.getSize().x, 500.f)
	, mSpawnPosition(mWorldView.getSize().x / 2.f, mWorldBounds.height - mWorldView.getSize().y / 2.f)
	, mScrollSpeed(0.f)
	, mPlayerCharacter(nullptr)
	, mEnemySpawnPoints()
	, mActiveEnemies()
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	loadTextures();
	buildScene();

	// Prepare the view
	mWorldView.setCenter(mSpawnPosition);
}

void World::update(sf::Time dt)
{

#pragma region Author: Alex

	// Scroll the world, reset player velocity
	//mWorldView.move(0.f, mScrollSpeed * dt.asSeconds());

	sf::Vector2f playerVelocity = mPlayerCharacter->getVelocity();

	// Stick view to player position
	mWorldView.setCenter(mPlayerCharacter->getPosition());
	/*
	Quick Alternative To:
	mWorldView.move(playerVelocity.x * dt.asSeconds(), playerVelocity.y * dt.asSeconds());
	*/

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

	// Check if player is out of bounds (bottom side)
	// getBattlefieldBounds().height + mWorldBounds.height = Height of battlefield added on with World bound height
	if (mPlayerCharacter->getPosition().y >= (getBattlefieldBounds().height + mWorldBounds.height))
	{
		mPlayerCharacter->setPosition(mPlayerCharacter->getPosition().x, (getBattlefieldBounds().height + mWorldBounds.height));
	}
	//mWorldView.move(playerVelocity.x * dt.asSeconds(), playerVelocity.y * dt.asSeconds());

#pragma endregion

	mPlayerCharacter->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide grenades
	destroyEntitiesOutsideView();
	guideGrenades();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);
	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();
	spawnEnemies();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();
	updateSounds();
}

void World::draw()
{
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear();
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
	mTextures.load(TextureIDs::Entities, "Media/Textures/Entities.png");
	mTextures.load(TextureIDs::Jungle, "Media/Textures/Jungle.png");
	mTextures.load(TextureIDs::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(TextureIDs::Particle, "Media/Textures/Particle.png");
	mTextures.load(TextureIDs::FinishLine, "Media/Textures/FinishLine.png");

	mTextures.load(TextureIDs::PlayerMove, "Media/Textures/PlayerMove.png");
	mTextures.load(TextureIDs::Grenade, "Media/Textures/Grenade.png");
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
	//if (velocity.x != 0.f && velocity.y != 0.f)
	//	mPlayerCharacter->setVelocity(velocity / std::sqrt(2.f));

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
	else if (static_cast<int>(type1) & category2 && static_cast<int>(type2) & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	for (SceneNode::Pair pair : collisionPairs)
	{
		if (matchesCategories(pair, Category::PlayerCharacter, Category::EnemyCharacter))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& enemy = static_cast<Character&>(*pair.second);

			// Collision: Player damage = enemy's remaining HP
			player.damage(enemy.getHitpoints());
			enemy.destroy();
		}

		else if (matchesCategories(pair, Category::PlayerCharacter, Category::Pickup))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& pickup = static_cast<Character&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			//pickup.apply(player);
			pickup.destroy();
			player.playLocalSound(mCommandQueue, SoundEffectIDs::CollectPickup);
		}

		else if (matchesCategories(pair, Category::EnemyCharacter, Category::AlliedProjectile)
			|| matchesCategories(pair, Category::PlayerCharacter, Category::EnemyProjectile))
		{
			auto& character = static_cast<Character&>(*pair.first);
			auto& projectile = static_cast<Character&>(*pair.second);

			// Apply projectile damage to Character, destroy projectile
			//character.damage(projectile.getDamage());
			//projectile.destroy();
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
	for (std::size_t i = 0; i < Layer::LayerCount; ++i)
	{
		Category category = (i == Layer::LowerAir) ? Category::SceneAirLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	// Prepare the tiled background
	sf::Texture& jungleTexture = mTextures.get(TextureIDs::Jungle);
	jungleTexture.setRepeated(true);

	float viewHeight = mWorldView.getSize().y;
	sf::IntRect textureRect(mWorldBounds);
	textureRect.height += static_cast<int>(viewHeight);

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> jungleSprite(new SpriteNode(jungleTexture, textureRect));
	jungleSprite->setPosition(mWorldBounds.left, mWorldBounds.top);
	mSceneLayers[Layer::Background]->attachChild(std::move(jungleSprite));

	// Add the finish line to the scene
	sf::Texture& finishTexture = mTextures.get(TextureIDs::FinishLine);
	std::unique_ptr<SpriteNode> finishSprite(new SpriteNode(finishTexture));
	finishSprite->setPosition(0.f, -76.f);
	mSceneLayers[Background]->attachChild(std::move(finishSprite));

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
	mSceneLayers[Layer::UpperAir]->attachChild(std::move(player));

	// Add enemy Character
	//addEnemies();
}

void World::addEnemies()
{
	// Add enemies to the spawn point container

	//addEnemy(Character::Type::Zombie, 0.f, 1000.f);
	//addEnemy(Character::Type::Zombie, +100.f, 1150.f);
	//addEnemy(Character::Type::Zombie, -100.f, 1150.f);
	//addEnemy(Character::Type::Avenger, 70.f, 1500.f);
	//addEnemy(Character::Type::Avenger, -70.f, 1500.f);
	//addEnemy(Character::Type::Avenger, -70.f, 1710.f);
	//addEnemy(Character::Type::Avenger, 70.f, 1700.f);
	//addEnemy(Character::Type::Avenger, 30.f, 1850.f);
	//addEnemy(Character::Type::Zombie, 300.f, 2200.f);
	//addEnemy(Character::Type::Zombie, -300.f, 2200.f);
	//addEnemy(Character::Type::Zombie, 0.f, 2200.f);
	//addEnemy(Character::Type::Zombie, 0.f, 2500.f);
	//addEnemy(Character::Type::Avenger, -300.f, 2700.f);
	//addEnemy(Character::Type::Avenger, -300.f, 2700.f);
	//addEnemy(Character::Type::Zombie, 0.f, 3000.f);
	//addEnemy(Character::Type::Zombie, 250.f, 3250.f);
	//addEnemy(Character::Type::Zombie, -250.f, 3250.f);
	//addEnemy(Character::Type::Avenger, 0.f, 3500.f);
	//addEnemy(Character::Type::Avenger, 0.f, 3700.f);
	//addEnemy(Character::Type::Zombie, 0.f, 3800.f);
	//addEnemy(Character::Type::Avenger, 0.f, 4000.f);
	//addEnemy(Character::Type::Avenger, -200.f, 4200.f);
	//addEnemy(Character::Type::Zombie, 200.f, 4200.f);
	//addEnemy(Character::Type::Zombie, 0.f, 4400.f);
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

		mSceneLayers[Layer::UpperAir]->attachChild(std::move(enemy));

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

void World::guideGrenades()
{
	// Setup command that stores all enemies in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = static_cast<int>(Category::EnemyCharacter);
	enemyCollector.action = derivedAction<Character>([this](Character& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveEnemies.push_back(&enemy);
	});

	// Setup command that guides all grenades to the enemy which is currently closest to the player
	Command grenadeGuider;
	grenadeGuider.category = static_cast<int>(Category::AlliedProjectile);
	grenadeGuider.action = derivedAction<Projectile>([this](Projectile& grenade, sf::Time)
	{
		// Ignore unguided bullets
		if (!grenade.isGuided())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Character* closestEnemy = nullptr;

		// Find closest enemy
		for (Character* enemy : mActiveEnemies)
		{
			float enemyDistance = distance(grenade, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			grenade.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(grenadeGuider);
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
	//bounds.top -= 100.f;
	//bounds.height += 100.f;

	return bounds;
}