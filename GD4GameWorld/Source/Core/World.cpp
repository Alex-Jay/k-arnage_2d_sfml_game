#include "Entity/Projectile.hpp"
#include "Entity/Pickup.hpp"
#include "Entity/Explosion.hpp"
#include "Structural/Foreach.hpp"
#include "Node/TextNode.hpp"
#include "Node/ParticleNode.hpp"
#include "Node/SoundNode.hpp"
#include "Node/NetworkNode.hpp" 
#include "Structural/Utility.hpp"
#include "Core/World.hpp"
#include "Entity/Obstacle.hpp"
#include "Constant/Constants.hpp"
#include "Structural/MapTiler.hpp"
#include "Effect/PostEffect.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>
#include <limits>



World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked)
	: mTarget(outputTarget)
	, mWorldView(outputTarget.getDefaultView())
	, mFonts(fonts)
	, mSounds(sounds)
	, mSceneLayers()
	, mZombieSpawnTime(-1)
	, mNumZombiesSpawn(2)
	, mNumZombiesAlive(0)
	, mZombieHitDelay(sf::Time::Zero)
	, mZombieHitElapsedTime(sf::Time::Zero)
	, mNetworkedWorld(networked)
	, mNetworkNode(nullptr)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);
	mWaterSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	// Alex - Apply zoom factor
	mWorldView.zoom(LEVEL_ZOOM_FACTOR);

	loadTextures();
	buildScene();

	//Sets the Distortion shaders texture
	mDistortionEffect.setTextureMap(mTextures);
}

void World::loadTextures()
{
	//TODO PLACE SEPARATE TEXTURES INTO SPRITE SHEETS, REMOVE unused ASSETS
	mTextures.load(Textures::ID::Entities, "Media/Textures/Entities.png");
	mTextures.load(Textures::ID::Water, "Media/Textures/Water.jpg");
	mTextures.load(Textures::ID::Lava, "Media/Textures/Lava.png");
	mTextures.load(Textures::ID::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(Textures::ID::Particle, "Media/Textures/Particle.png");
	mTextures.load(Textures::ID::PlayerMove, "Media/Textures/PlayerMove.png");
	mTextures.load(Textures::ID::PlayerDeath, "Media/Textures/Blood.png");
	mTextures.load(Textures::ID::ZombieMove, "Media/Textures/ZombieWalk.png");
	mTextures.load(Textures::ID::ZombieDeath, "Media/Textures/ZombieDeath.png");
	mTextures.load(Textures::ID::Grenade, "Media/Textures/Grenade.png");
	mTextures.load(Textures::ID::MapTiles, "Media/Textures/Tiles.png");
	mTextures.load(Textures::ID::Crate, "Media/Textures/Crate.png");
	mTextures.load(Textures::ID::DistortionMap, "Media/Textures/distortion_map.png");
	mTextures.load(Textures::ID::PlaneWreck, "Media/Textures/PlaneWreck1.png");
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


#pragma endregion

#pragma region Update

void World::update(sf::Time dt)
{
	//std::cout << "CHARACTERS IN WORLD : " << mPlayerCharacters.size() << std::endl;
	setView();

	// Setup commands to destroy entities, and guide grenades
	destroyEntitiesOutsideView();

	//TOFIX This Should Be handled By Server
	guideZombies();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);

	// Collision detection and response (may destroy entities)
	handleCollisions(dt);

	// Remove characters that were destroyed (World::removeWrecks() only destroys the entities, not the pointers in mPlayerCharacter)
	auto firstToRemove = std::remove_if(mPlayerCharacters.begin(), mPlayerCharacters.end(), std::mem_fn(&Character::isMarkedForRemoval));
	mPlayerCharacters.erase(firstToRemove, mPlayerCharacters.end());

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();


	addZombies(dt);
	spawnZombies();

	spawnObstacles();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);

	updateSounds();

}

void World::draw()
{
	//If PostEffect is NOT supported, the water background is added to the scenGraph so everything is still drawn
	//Otherwise it is seperated from the sceneGraph to apply post effects seperetly
	if (PostEffect::isSupported())
	{

		mWaterSceneTexture.clear();
		mWaterSceneTexture.setView(mWorldView);
		mWaterSceneTexture.draw(mWaterSprite);
		mWaterSceneTexture.display();
		mDistortionEffect.apply(mWaterSceneTexture, mTarget);
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
	else
	{
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
}

void World::setView()
{
	for (Character* c : mPlayerCharacters)
	{
		if (c->getLocalIdentifier() == localCharacterID)
		{
			mWorldView.setCenter(c->getPosition());
		}
		c->setVelocity(0.f, 0.f);
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

int8_t World::getAliveZombieCount()
{
	return mNumZombiesAlive;
}

void World::setAliveZombieCount(int8_t count)
{
	mNumZombiesAlive = count;
}

Character * World::getCharacter(int8_t localIdentifier) const
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

void World::removeCharacter(int8_t localIdentifier)
{
	Character* character = getCharacter(localIdentifier);
	if (character)
	{
		character->destroy();
		mPlayerCharacters.erase(std::find(mPlayerCharacters.begin(), mPlayerCharacters.end(), character));
	}
}

Character * World::addCharacter(int8_t identifier, bool isLocal)
{

	std::unique_ptr<Character> player(new Character(Character::Type::Player, mTextures, mFonts));
	player->setPosition(mWorldView.getCenter());
	player->setLocalIdentifier(identifier);

	mPlayerCharacters.push_back(player.get());
	mSceneLayers[Layer::UpperLayer]->attachChild(std::move(player));

	if (isLocal)
	{
		localCharacterID = (identifier);
	}

	return mPlayerCharacters.back();
}

bool World::hasAlivePlayer() const
{
	return mPlayerCharacters.size() > 0;
}

void World::updateSounds()
{
	//// Set listener's position to player position
	//mSounds.setListenerPosition(mPlayerCharacter->getWorldPosition());

	sf::Vector2f listenerPosition;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (mPlayerCharacters.empty())
		listenerPosition = mWorldView.getCenter();

	// 1 or more players -> mean position between all characters
	else
	{
		for (Character* character : mPlayerCharacters)
			listenerPosition += character->getWorldPosition();

		listenerPosition /= static_cast<float>(mPlayerCharacters.size());
	}

	// Set listener's position
	mSounds.setListenerPosition(listenerPosition);

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

//Mike
void World::addZombies(sf::Time dt)
{
	if (mNetworkedWorld)
		return;

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

				float angle = 0;// -mPlayerCharacters[localCharacterID]->getAngle();

				std::unique_ptr<Character> enemy(new Character(Character::Type::Zombie, mTextures, mFonts));
				enemy->setPosition(xPos, yPos);
				enemy->setRotation(angle);

				if (shrink(mWorldBoundsBuffer, mWorldBounds).intersects(enemy->getBoundingRect()))
				{
					//mSceneLayers[UpperLayer]->attachChild(std::move(enemy));
					addZombie(xPos, yPos, angle);
					++mNumZombiesAlive;
				}
			}
		}

		mZombieSpawnTimerStarted = false;
	}
}

void World::addZombie(int16_t x, int16_t y, int16_t a)
{
	SpawnPoint spawn(Character::Type::Zombie, x, y, a);
	mEnemySpawnPoints.push_back(spawn);
	//std::cout << "SPAWN POINT ADDED " << std::endl;
}

void World::addObstacle(Obstacle::ObstacleID type, int16_t x, int16_t y, int16_t a)
{
	obstacleSpawnPoint spawn(type, x, y, a);
	mObstacleSpawnPoints.push_back(spawn);
}

void World::spawnZombies()
{
	// Spawn all enemies entering the view area (including distance) this frame
	while (!mEnemySpawnPoints.empty())
	{
		SpawnPoint spawn = mEnemySpawnPoints.back();

		std::unique_ptr<Character> enemy(new Character(spawn.type, mTextures, mFonts));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(180.f);
		//if (mNetworkedWorld) enemy->disablePickups();

		mSceneLayers[UpperLayer]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();

		++mNumZombiesAlive;

		//std::cout << "Spawned Zombie at X: " << spawn.x << " Y: " << spawn.y << std::endl;
		//std::cout << "Zombies Alive World: " << mNumZombiesAlive << std::endl;
	}
}

void World::spawnObstacles()
{
	// Spawn all enemies entering the view area (including distance) this frame
	while (!mObstacleSpawnPoints.empty()
		&& mObstacleSpawnPoints.back().y > getBattlefieldBounds().top)
	{
		obstacleSpawnPoint spawn = mObstacleSpawnPoints.back();

		std::unique_ptr<Obstacle> enemy(new Obstacle(spawn.type, mTextures));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(spawn.a);
		//if (mNetworkedWorld) enemy->disablePickups();

		mSceneLayers[UpperLayer]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mObstacleSpawnPoints.pop_back();
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
	mCommandQueue.push(destroyProjectiles);

	if (mNetworkedWorld)
		return;

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
		Category::Type category = i == LowerLayer ? Category::SceneLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	std::unique_ptr<MapTiler> map(new MapTiler(MapTiler::MapID::Dessert, mTextures));
	mWorldBounds = map->getMapBounds();
	mWorldBoundsBuffer = map->getTileSize().x;

	mWaterTexture = mTextures.get(Textures::ID::Lava);
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


	addObstacles();

	//spawnZombies(sf::Time::Zero);
	mZombieSpawnTime = 10;//Spawn Every 10 seconds

	// Add network node, if necessary
	if (mNetworkedWorld) 
	{
		std::unique_ptr<NetworkNode> networkNode(new NetworkNode());
		mNetworkNode = networkNode.get();
		mSceneGraph.attachChild(std::move(networkNode));
	}
}

//Mike
void World::addObstacles()
{
	if (mNetworkedWorld)
		return;

	//Random Number of Obstacles to spawn
	int rand = randomInt(20);

	//List for all Spawned Bounding rects
	std::list<sf::FloatRect> objectRects;
	Obstacle::ObstacleID oType;
	for (int i = 0; i < rand; i++)
	{
		int type = randomInt(20);

		if (type < 10)
			oType = Obstacle::ObstacleID::Wreck;
		else
			oType = Obstacle::ObstacleID::Crate;

		//Random Position in the world
		int xPos = randomInt(std::ceil(mWorldBounds.width));
		int yPos = randomInt(std::ceil(mWorldBounds.height));

		std::unique_ptr<Obstacle> obstacle(new Obstacle(oType, mTextures));
		obstacle->setPosition(sf::Vector2f(xPos, yPos));
		obstacle->setRotation(type * 18);
		sf::FloatRect boundingRectangle = obstacle->getBoundingRect();

		//If the list does not contain that rectangle spawn a new object
		if (!containsIntersection(objectRects, boundingRectangle)
			&& shrink((mWorldBoundsBuffer * 2), mWorldBounds).contains(sf::Vector2f(xPos, yPos)))
		{
			objectRects.push_back(boundingRectangle);
			addObstacle(oType, xPos, yPos, type * 18);
			//mSceneLayers[UpperLayer]->attachChild(std::move(obstacle));
		}
	}
}

#pragma endregion

#pragma region Collisions

bool matchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
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
	FOREACH(Character* c, mPlayerCharacters)
		if (c->getLocalIdentifier() == localCharacterID
			&& !mWorldBounds.contains(c->getPosition()))
			mPlayerCharacters[localCharacterID]->damage(LAVA_DAMAGE);
}

//Mike
void World::handleCollisions(sf::Time dt)
{
	handlePlayerCollision();

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
	//TODO WAY To MANY if Else Going On here, can be refactored much cleaner
	if (projectile.isGrenade())
	{
		if (Character* player = dynamic_cast<Character*>(&entity))
		{
			if (!player->getLocalIdentifier() == localCharacterID)
			{
				projectile.setVelocity(-projectile.getVelocity() / 2.f);
			}
		}
		else
			projectile.setVelocity(-projectile.getVelocity() / 2.f);
	}
	else
	{
		if (Obstacle* o = dynamic_cast<Obstacle*>(&entity))
		{
			if (o->isDestructible())
			{
				entity.damage(projectile.getDamage());
			}
		}
		else
		{
			entity.damage(projectile.getDamage());
		}
		

		if (entity.isDestroyed())
		{
			// Alex - If entity can be dynamically casted to a Character [Zombie not a an Obstacle] when it's destroyed,
			// decrement zombie alive count and increment player score accordingly
			if (Character* zombie = dynamic_cast<Character*>(&entity))
			{
				int currZombiesAlive = getAliveZombieCount();
				setAliveZombieCount(--currZombiesAlive);

				//TOD INcrement Player Score Based on Player ID From Bullet
				// Increment Player 1 Score
				if (projectile.getProjectileId() == 0)
				{
					//incrementPlayerOneScore(ZOMBIE_KILL_MULTIPLIER);
				}
				// Increment Player 2 Score
				else if (projectile.getProjectileId() == 1)
				{
					//incrementPlayerTwoScore(ZOMBIE_KILL_MULTIPLIER);
				}
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

	player.playLocalSound(mCommandQueue, SoundEffect::ID::CollectPickup);
}

//Mike
void World::handleExplosionCollisions(SceneNode::Pair& pair)
{
	auto& entity = static_cast<Entity&>(*pair.first);
	auto& explosion = static_cast<Explosion&>(*pair.second);

	entity.damage(10);
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
		|| matchesCategories(pair, Category::PlayerCharacter, Category::AlliedProjectile))
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


bool World::pollGameAction(GameActions::Action& out)
{
	return mNetworkNode->pollGameAction(out);
}

void World::createPickup(sf::Vector2f position, Pickup::Type type)
{
	std::unique_ptr<Pickup> pickup(new Pickup(type, mTextures));
	pickup->setPosition(position);
	pickup->setVelocity(0.f, 1.f);
	mSceneLayers[UpperLayer]->attachChild(std::move(pickup));
}

#pragma endregion


