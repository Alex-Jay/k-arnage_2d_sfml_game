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
	  , mZombieSpawnTime(-1)
	  , mNumZombiesSpawn(2)
	  , mNumZombiesAlive(0)
	  , mZombieHitDelay(sf::Time::Zero)
	  , mZombieHitElapsedTime(sf::Time::Zero)
	  , mPlayerCharacter(nullptr)
	  //, mPlayerTwoCharacter(nullptr)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);
	mWaterSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	// Alex - Apply zoom factor
	mWorldView.zoom(LEVEL_ZOOM_FACTOR);

	loadTextures();
	
	buildScene();

	//Sets the Distortion shaders texture
	mDistortionEffect.setTextureMap(mTextures);

	// Add Local Player on Start
	mPlayerCharacter = addCharacter(0);
	//mPlayerTwoCharacter = addCharacter(1);
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

	mTextures.load(TextureIDs::DistortionMap, "Media/Textures/distortion_map.png");
}

#pragma region Mutators

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

// Zombie Hit Delay Mutators
sf::Time World::getZombieHitDelay()
{
	return mZombieHitDelay;
}

void World::setZombieHitDelay(sf::Time delay)
{
	mZombieHitDelay = delay;
}

sf::Time World::getZombieHitElapsedTime()
{
	return mZombieHitElapsedTime;
}

void World::incrementZombieHitElapsedTime(sf::Time dt)
{
	mZombieHitElapsedTime += dt;
}

void World::resetZombieHitElapsedTime()
{
	mZombieHitElapsedTime = sf::Time::Zero;
}

unsigned int const World::getPlayerOneScore() const
{
	return mPlayerOneScore;
}

void World::incrementPlayerOneScore(unsigned int incBy)
{
	mPlayerOneScore += incBy;
}

#pragma endregion

#pragma region Update

void World::update(sf::Time dt)
{
	// Alex - Stick view to player position
	mWorldView.setCenter(getCharacter(0)->getPosition());

	/*
	Quick Alternative To:
	mWorldView.move(playerVelocity.x * dt.asSeconds(), playerVelocity.y * dt.asSeconds());
	*/

	// Alex - Handle player collisions (e.g. prevent leaving battlefield)
	handlePlayerCollision();

	for (Character* c : mPlayerCharacters)
	{
		c->setVelocity(0.f, 0.f);
	}

	//mPlayerCharacter->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide grenades
	destroyEntitiesOutsideView();
	guideZombies();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);

	//adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions(dt);

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);

	adaptPlayerPosition();
	updateSounds();
	spawnZombies(dt);

}

void World::draw()
{
	//If PostEffect is NOT supported, the water background is added to the scenGraph so everything is still drawn
	//Otherwise it is seperated from the sceneGraph to apply post effects seperetly
	if (PostEffect::isSupported())
	{

		mWaterSceneTexture.clear();
		//mSceneTexture.clear();

		//Apply distortion Shader to SpriteNode(Water) Background, this is seperated from the sceneGraph
		mWaterSceneTexture.setView(mWorldView);
		mWaterSceneTexture.draw(mWaterSprite);
		mWaterSceneTexture.display();
		mDistortionEffect.apply(mWaterSceneTexture, mTarget);

		//Apply BloomEffect to the sceneGraph that does not contain the water background.
		//mSceneTexture.setView(mWorldView);
		//mSceneTexture.draw(mSceneGraph);
		//mSceneTexture.display();
		//mBloomEffect.apply(mSceneTexture, mTarget);


		//Draw Scenegraph on top of water background with no bloom effect.
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
	else
	{

		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
}

// Alex - Get collision mainfold
sf::Vector3f World::getMainfold(const sf::FloatRect & overlap, const sf::Vector2f & collisionNormal)
{
	/*
		X: Collision X-Axis Normal
		Y: Collision Y-Axis Normal
		Z: Penetration Amount
	*/

	sf::Vector3f mainfold;

	// Determine Axis of collision
	// X-Axis
	if (overlap.width < overlap.height)
	{
		mainfold.x = (collisionNormal.x > 0) ? 1.f : -1.f;
		mainfold.z = overlap.width;
	}
	// Y-Axis
	else
	{
		mainfold.y = (collisionNormal.y > 0) ? 1.f : -1.f;
		mainfold.z = overlap.height;
	}

	return mainfold;
}

int World::getAliveZombieCount()
{
	return mNumZombiesAlive;
}

void World::setAliveZombieCount(int count)
{
	mNumZombiesAlive = count;
}

Character * World::getCharacter(int localIdentifier) const
{
	for (Character* c : mPlayerCharacters)
	{
		if (c->getLocalIdentifier() == localIdentifier)
		{
			return c;
		}
	}
	return nullptr;
}

void World::removeCharacter(int localIdentifier)
{
	Character* character = getCharacter(localIdentifier);
	if (character)
	{
		character->destroy();
		mPlayerCharacters.erase(std::find(mPlayerCharacters.begin(), mPlayerCharacters.end(), character));
	}
}

Character * World::addCharacter(int localIdentifier)
{
	std::unique_ptr<Character> player(new Character(Character::Type::Player, mTextures, mFonts));
	player->setPosition(mWorldView.getCenter());
	player->setLocalIdentifier(localIdentifier);

	mPlayerCharacters.push_back(player.get());
	mSceneLayers[Layer::UpperLayer]->attachChild(std::move(player));
	return mPlayerCharacters.back();
}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	//sf::FloatRect viewBounds = getViewBounds();
	//const float borderDistance = 40.f;

	//sf::Vector2f position = mPlayerCharacter->getPosition();
	//position.x = std::max(position.x, viewBounds.left + borderDistance);
	//position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
	//position.y = std::max(position.y, viewBounds.top + borderDistance);
	//position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
	//mPlayerCharacter->setPosition(position);

	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	for(Character* character : mPlayerCharacters)
	{
		sf::Vector2f position = character->getPosition();
		position.x = std::max(position.x, viewBounds.left + borderDistance);
		position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
		position.y = std::max(position.y, viewBounds.top + borderDistance);
		position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
		character->setPosition(position);
	}
}

void World::adaptPlayerVelocity()
{
	//sf::Vector2f velocity = mPlayerCharacter->getVelocity();

	//// If moving diagonally, reduce velocity (to have always same velocity)
	//if (velocity.x != 0.f && velocity.y != 0.f)
	//	mPlayerCharacter->setVelocity(velocity / std::sqrt(2.f));

	//// Add scrolling velocity
	//mPlayerCharacter->accelerate(0.f, mScrollSpeed);

	for(Character* character : mPlayerCharacters)
	{
		sf::Vector2f velocity = character->getVelocity();

		// If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
			character->setVelocity(velocity / std::sqrt(2.f));

		// Add scrolling velocity
		//character->accelerate(0.f, mScrollSpeed);
	}
}

bool World::hasAlivePlayer() const
{
	return !mPlayerCharacter->isMarkedForRemoval();
	/*return mPlayerCharacters.size() > 0;*/
}

bool World::hasPlayerReachedEnd() const
{
	if (Character* character = getCharacter(1))
		return !mWorldBounds.contains(character->getPosition());
	else
		return false;
}

void World::updateSounds()
{
	//// Set listener's position to player position
	//mSounds.setListenerPosition(mPlayerCharacter->getWorldPosition());

	//// Remove unused sounds
	//mSounds.removeStoppedSounds();

	sf::Vector2f listenerPosition;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (mPlayerCharacters.empty())
	{
		listenerPosition = mWorldView.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		for(Character* character : mPlayerCharacters)
			listenerPosition += character->getWorldPosition();

		listenerPosition /= static_cast<float>(mPlayerCharacters.size());
	}

	// Set listener's position
	mSounds.setListenerPosition(listenerPosition);

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

//Mike
void World::spawnZombies(sf::Time dt)
{
	if (!mZombieSpawnTimerStarted)
	{
		StartZombieSpawnTimer(dt);
	}
	else
	{
		mZombieSpawnTimer += dt;
	}

	if (mZombieSpawnTimer.asSeconds() >= mZombieSpawnTime
		&& getAliveZombieCount() < 20)
	{
			for (int i = 0; i < mNumZombiesSpawn; ++i)
			{
				//Picks A random position outside the view but within world bounds
				int xPos = randomIntExcluding(std::ceil(getViewBounds().left), std::ceil(getViewBounds().width));
				int yPos = randomIntExcluding(std::ceil(getViewBounds().top), std::ceil(getViewBounds().height));

				std::unique_ptr<Character> enemy(new Character(Character::Type::Zombie, mTextures, mFonts));
				enemy->setPosition(xPos, yPos);
				enemy->setRotation(180.f);

				if (shrink(mWorldBoundsBuffer, mWorldBounds).intersects(enemy->getBoundingRect()))
				{
					//Picks A random position outside the view but within world bounds
					int xPos = randomIntExcluding(std::ceil(getViewBounds().left), std::ceil(getViewBounds().width));
					int yPos = randomIntExcluding(std::ceil(getViewBounds().top), std::ceil(getViewBounds().height));

					std::unique_ptr<Character> enemy(new Character(Character::Type::Zombie, mTextures, mFonts));
					enemy->setPosition(xPos, yPos);
					enemy->setRotation(-mPlayerCharacter->getAngle());

					if (shrink(mWorldBoundsBuffer, mWorldBounds).intersects(enemy->getBoundingRect()))
					{
						mSceneLayers[UpperLayer]->attachChild(std::move(enemy));
						++mNumZombiesAlive;
					}
				}
			}
		
		mZombieSpawnTimerStarted = false;
	}
}

//Mike
void World::StartZombieSpawnTimer(sf::Time dt)
{
	mZombieSpawnTimerStarted = true;
	mZombieSpawnTimer = dt;
}

//Mike
void World::destroyEntitiesOutsideView()
{
	// Destroy Projectiles outside of view
	Command destroyProjectiles;
	destroyProjectiles.category = static_cast<int>(Category::Projectile);
	destroyProjectiles.action = derivedAction<Entity>([this](Entity& e, sf::Time)
	{
		if (!getBattlefieldBounds().intersects(e.getBoundingRect()))
			e.destroy();
	});

	//Destroy Zombies Outside of world Bounds
	Command destroyZombies;
	destroyZombies.category = static_cast<int>(Category::EnemyCharacter);
	destroyZombies.action = derivedAction<Entity>([this](Entity& e, sf::Time)
	{
		if (!mWorldBounds.intersects(e.getBoundingRect()))
		{
			e.destroy();
			int currZombiesAlive = getAliveZombieCount();
			setAliveZombieCount(--currZombiesAlive);
		}
	});

	mCommandQueue.push(destroyProjectiles);
	mCommandQueue.push(destroyZombies);
}

//Mike
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
	mWorldBoundsBuffer = map->getTileSize().x;

	mWaterTexture = mTextures.get(TextureIDs::Water);
	mWaterTexture.setRepeated(true);

	float viewHeight = mWorldBounds.height;
	float viewWidth = mWorldBounds.width;

	sf::IntRect textureRect(sf::FloatRect(mWorldBoundsBuffer, mWorldBoundsBuffer, mWorldBounds.width * 2, mWorldBounds.height * 2));
	textureRect.height += static_cast<int>(viewHeight);

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> waterSprite(new SpriteNode(mWaterTexture, textureRect));
	waterSprite->setPosition(-viewWidth / 2, -viewHeight);

	if (PostEffect::isSupported())
	{
		mWaterSprite.attachChild(std::move(waterSprite));
	}
	else
	{
		mSceneLayers[Background]->attachChild(std::move(waterSprite));
	}

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
	//std::unique_ptr<Character> player(new Character(Character::Type::Player, mTextures, mFonts));
	//mPlayerCharacter = player.get();


	//mPlayerCharacter->setPosition(getCenter(mWorldBounds));
	//mSceneLayers[UpperLayer]->attachChild(std::move(player));

	SpawnObstacles();

	//spawnZombies(sf::Time::Zero);
	mZombieSpawnTime = 10;//Spawn Every 10 seconds
}

//Mike
void World::SpawnObstacles()
{
	//Random Number of Obstacles to spawn
	int rand = randomInt(20);

	//List for all Spawned Bounding rects
	std::list<sf::FloatRect> objectRects;

	for (int i = 0; i < rand; i++)
	{
		//Random Position in the world
		int xPos = randomInt(std::ceil(mWorldBounds.width));
		int yPos = randomInt(std::ceil(mWorldBounds.height));

		std::unique_ptr<Obstacle> obstacle(new Obstacle(Obstacle::ObstacleID::Crate, mTextures));
		obstacle->setPosition(sf::Vector2f(xPos, yPos));
		sf::FloatRect boundingRectangle = obstacle->getBoundingRect();

		//If the list does not contain that rectangle spawn a new object
		if (!containsIntersection(objectRects, boundingRectangle)
			&& shrink((mWorldBoundsBuffer * 2), mWorldBounds).contains(sf::Vector2f(xPos, yPos)))
		{
			objectRects.push_back(boundingRectangle);
			mSceneLayers[UpperLayer]->attachChild(std::move(obstacle));
		}
	}	
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

//Mike
void World::handlePlayerCollision()
{
	// Map bound collision
	if (!shrink(mWorldBoundsBuffer, mWorldBounds).contains(mPlayerCharacter->getPosition()))
	{
		mPlayerCharacter->setPosition(mPlayerCharacter->getLastPosition());
	}
	//else if (!shrink(mWorldBoundsBuffer, mWorldBounds).contains(mPlayerTwoCharacter->getPosition()))
	//{
	//	mPlayerTwoCharacter->setPosition(mPlayerTwoCharacter->getLastPosition());
	//}
}

//Mike
void World::handleCollisions(sf::Time dt)
{	
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	for (SceneNode::Pair pair : collisionPairs)
	{
		CollisionType collisionType = GetCollisionType(pair);
		switch (collisionType)
		{
			case Character_Character:
				incrementZombieHitElapsedTime(dt);
				handleCharacterCollisions(pair);
				break;
			case Player_Pickup:
				handlePickupCollisions(pair);
				break;
			case Character_Obstacle:
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

//Mike
void World::handleCharacterCollisions(SceneNode::Pair& pair)
{
	auto& character1 = static_cast<Character&>(*pair.first);
	auto& character2 = static_cast<Character&>(*pair.second);

	sf::FloatRect r1 = character1.getBoundingRect();
	sf::FloatRect r2 = character2.getBoundingRect();
	sf::FloatRect intersection;

	// Get collision intersection FloatRect
	if (r1.intersects(r2, intersection))
	{

		// Get distance between two characters
		sf::Vector2f diff = character1.getWorldPosition() - character2.getWorldPosition();

		/*
			Get mainfold floatrect of colliding bodies
			Return Type: Vector3f
			X = Collision X-Axis Normal
			Y = Collision Y-Axis Normal
			Z = Penetration Amount
		*/
		sf::Vector3f mainfold = getMainfold(intersection, diff);


		// Normal vector to move collidee out of the hitbox
		sf::Vector2f normal(mainfold.x, mainfold.y);

		// Assert an Entity cast can be made
		assert(dynamic_cast<Entity*>(&character1) != nullptr);
		assert(dynamic_cast<Entity*>(&character2) != nullptr);

		// Normal vector * mainfold.z = Moving back by amount of penetration
		// Normal & -Normal moves collidee's away from eachother
		static_cast<Entity&>(character1).move(normal * mainfold.z);
		static_cast<Entity&>(character2).move(-normal * mainfold.z);
	}

	if ((character1.isZombie() && character2.isZombie())
		|| (character1.isPlayer() && character2.isPlayer()))
	{
		//DONE FIX COLLISION
	}
	else
	{
		if (getZombieHitElapsedTime() >= sf::milliseconds(ZOMBIEATTACKDELAY))
		{
			if (character1.isZombie())
				character2.damage(ZOMBIEATTACKDAMAGE);
			else
				character1.damage(ZOMBIEATTACKDAMAGE);

			resetZombieHitElapsedTime();
		}
	}
}

//Mike
void World::handleObstacleCollisions(SceneNode::Pair& pair)
{
	auto& character = static_cast<Character&>(*pair.first);
	auto& obstacle = static_cast<Obstacle&>(*pair.second);

	sf::FloatRect r1 = character.getBoundingRect();
	sf::FloatRect r2 = obstacle.getBoundingRect();
	sf::FloatRect intersection;

	if (r1.intersects(r2, intersection))
	{
		sf::Vector2f diff = character.getWorldPosition() - obstacle.getWorldPosition();

		sf::Vector3f mainfold = getMainfold(intersection, diff);

		sf::Vector2f normal(mainfold.x, mainfold.y);

		assert(dynamic_cast<Entity*>(&character) != nullptr);
		static_cast<Entity&>(character).move(normal * mainfold.z);
	}
}

//Mike
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

		if (entity.isDestroyed())
		{
			// Alex - If entity can be dynamically casted to a Character [Zombie not a an Obstacle] when it's destroyed,
			// decrement zombie alive count and increment player score accordingly
			if (Character* zombie = dynamic_cast<Character*>(&entity))
			{
				int currZombiesAlive = getAliveZombieCount();
				setAliveZombieCount(--currZombiesAlive);

				// Increment Player 1 Score
				if (projectile.getProjectileId() == 0)
				{
					incrementPlayerOneScore(ZOMBIE_KILL_MULTIPLIER);
				}
				//// Increment Player 2 Score
				//else if (projectile.getProjectileId() == 1)
				//{
				//	incrementPlayerTwoScore(ZOMBIE_KILL_MULTIPLIER);
				//}
			}
		}
		projectile.destroy();
	}
}

//Mike
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

//Mike
void World::handleExplosionCollisions(SceneNode::Pair& pair)
{
	auto& entity = static_cast<Entity&>(*pair.first);
	auto& explosion = static_cast<Explosion&>(*pair.second);

	// Distance between characters and explosions center-points
	//float dV = vectorDistance(entity.getWorldPosition(), explosion.getWorldPosition());

	entity.damage(1);
}

//Mike
World::CollisionType World::GetCollisionType(SceneNode::Pair& pair)
{
	if (matchesCategories(pair, Category::PlayerCharacter, Category::EnemyCharacter)
		|| matchesCategories(pair, Category::EnemyCharacter, Category::EnemyCharacter)
		|| matchesCategories(pair, Category::PlayerCharacter, Category::PlayerCharacter))
	{
		return Character_Character;
	}
	else if (matchesCategories(pair, Category::PlayerCharacter, Category::Pickup))
	{
		return Player_Pickup;
	}
	else if (matchesCategories(pair, Category::PlayerCharacter, Category::Obstacle)
		|| matchesCategories(pair, Category::EnemyCharacter, Category::Obstacle))
	{
		return Character_Obstacle;
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