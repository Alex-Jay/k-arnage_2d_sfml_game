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
	, mSpawnPosition(mWorldView.getCenter())
	, mScrollSpeed(0.f)
	, mZombieSpawnTime(-1)
	, mNumZombiesSpawn(2)
	, mNumZombiesAlive(0)
	, mZombieHitDelay(sf::Time::Zero)
	, mZombieHitElapsedTime(sf::Time::Zero)
	//, mPlayerOneCharacter(nullptr)
	//, mPlayerTwoCharacter(nullptr)
	, mNetworkedWorld(networked)
	, mNetworkNode(nullptr)

{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);
	mWaterSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);
	mWorldView.zoom(LEVEL_ZOOM_FACTOR);

	loadTextures();
	buildScene();

	//Sets the Distortion shaders texture
	mDistortionEffect.setTextureMap(mTextures);

	// Prepare the view
	//mWorldView.setCenter(mSpawnPosition);

		// Add Local Player on Start
	//mPlayerOneCharacter = addCharacter(0);
	//mPlayerTwoCharacter = addCharacter(1);
}

void World::setWorldScrollCompensation(float compensation)
{
	mScrollSpeedCompensation = compensation;
}

void World::update(sf::Time dt)
{
	mWorldView.setCenter(getAircraft(1)->getPosition());

	handlePlayerCollision();

	FOREACH(Aircraft* a, mPlayerAircrafts)
		a->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide missiles
	destroyEntitiesOutsideView();
	guideZombies();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);

	//adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions(dt);

	// Remove aircrafts that were destroyed (World::removeWrecks() only destroys the entities, not the pointers in mPlayerAircraft)
	auto firstToRemove = std::remove_if(mPlayerAircrafts.begin(), mPlayerAircrafts.end(), std::mem_fn(&Aircraft::isMarkedForRemoval));
	mPlayerAircrafts.erase(firstToRemove, mPlayerAircrafts.end());

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();
	spawnZombies(dt);

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();

	updateSounds();
	//spawnZombies(dt);
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

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}

Aircraft* World::getAircraft(int identifier) const
{
	FOREACH(Aircraft* a, mPlayerAircrafts)
	{
		if (a->getIdentifier() == identifier)
			return a;
	}

	return nullptr;
}

void World::removeAircraft(int identifier)
{
	Aircraft* aircraft = getAircraft(identifier);
	if (aircraft)
	{
		aircraft->destroy();
		mPlayerAircrafts.erase(std::find(mPlayerAircrafts.begin(), mPlayerAircrafts.end(), aircraft));
	}
}

Aircraft* World::addAircraft(int identifier)
{
	std::unique_ptr<Aircraft> player(new Aircraft(Aircraft::Eagle, mTextures, mFonts));
	player->setPosition(mWorldView.getCenter());
	player->setIdentifier(identifier);

	mPlayerAircrafts.push_back(player.get());
	mSceneLayers[UpperAir]->attachChild(std::move(player));
	return mPlayerAircrafts.back();
}

void World::createPickup(sf::Vector2f position, Pickup::Type type)
{	
	std::unique_ptr<Pickup> pickup(new Pickup(type, mTextures));
	pickup->setPosition(position);
	pickup->setVelocity(0.f, 1.f);
	mSceneLayers[UpperAir]->attachChild(std::move(pickup));
}

bool World::pollGameAction(GameActions::Action& out)
{
	return mNetworkNode->pollGameAction(out);
}

void World::setCurrentBattleFieldPosition(float lineY)
{
	mWorldView.setCenter(mWorldView.getCenter().x, lineY - mWorldView.getSize().y/2);
	mSpawnPosition.y = mWorldBounds.height; 
}

void World::setWorldHeight(float height)
{
	mWorldBounds.height = height;
}

bool World::hasAlivePlayer() const
{
	return mPlayerAircrafts.size() > 0;
}

bool World::hasPlayerReachedEnd() const
{
	//if (Aircraft* aircraft = getAircraft(1))
	//	return !mWorldBounds.contains(aircraft->getPosition());
	//else 
		return false;
}

void World::loadTextures()
{
	mTextures.load(Textures::Entities, "Media/Textures/Entities.png");
	mTextures.load(Textures::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(Textures::Particle, "Media/Textures/Particle.png");
	mTextures.load(Textures::Water, "Media/Textures/Water.jpg");
	mTextures.load(Textures::PlayerMove, "Media/Textures/PlayerMove.png");
	mTextures.load(Textures::PlayerDeath, "Media/Textures/Blood.png");
	mTextures.load(Textures::ZombieMove, "Media/Textures/ZombieWalk.png");
	mTextures.load(Textures::ZombieDeath, "Media/Textures/ZombieDeath.png");
	mTextures.load(Textures::Grenade, "Media/Textures/Grenade.png");
	mTextures.load(Textures::MapTiles, "Media/Textures/Tiles.png");
	mTextures.load(Textures::Crate, "Media/Textures/Crate.png");
	mTextures.load(Textures::DistortionMap, "Media/Textures/distortion_map.png");
}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	FOREACH(Aircraft* aircraft, mPlayerAircrafts)
	{
		sf::Vector2f position = aircraft->getPosition();
		position.x = std::max(position.x, viewBounds.left + borderDistance);
		position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
		position.y = std::max(position.y, viewBounds.top + borderDistance);
		position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
		aircraft->setPosition(position);
	}
}

void World::adaptPlayerVelocity()
{
	FOREACH(Aircraft* aircraft, mPlayerAircrafts)
	{
		sf::Vector2f velocity = aircraft->getVelocity();

		// If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
			aircraft->setVelocity(velocity / std::sqrt(2.f));

		// Add scrolling velocity
		//aircraft->accelerate(0.f, mScrollSpeed);
	}
}

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


void World::updateSounds()
{
	sf::Vector2f listenerPosition;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (mPlayerAircrafts.empty())
	{
		listenerPosition = mWorldView.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		FOREACH(Aircraft* aircraft, mPlayerAircrafts)
			listenerPosition += aircraft->getWorldPosition();

		listenerPosition /= static_cast<float>(mPlayerAircrafts.size());
	}

	// Set listener's position
	mSounds.setListenerPosition(listenerPosition);

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

void World::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		Category::Type category = (i == LowerAir) ? Category::SceneLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	std::unique_ptr<MapTiler> map(new MapTiler(MapTiler::MapID::Dessert, mTextures));
	mWorldBounds = map->getMapBounds();
	mWorldBoundsBuffer = map->getTileSize().x;

	mWaterTexture = mTextures.get(Textures::ID::Water);
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
	mSceneLayers[LowerAir]->attachChild(std::move(smokeNode));

	// Add exhaust particle node to the scene
	std::unique_ptr<ParticleNode> exhaustNode(new ParticleNode(Particle::Type::Exhaust, mTextures));
	mSceneLayers[LowerAir]->attachChild(std::move(exhaustNode));

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

	// Add network node, if necessary
	if (mNetworkedWorld)
	{
		std::unique_ptr<NetworkNode> networkNode(new NetworkNode());
		mNetworkNode = networkNode.get();
		mSceneGraph.attachChild(std::move(networkNode));
	}
}

void World::addEnemies()
{
	if (mNetworkedWorld)
		return;

	// Add enemies to the spawn point container
	addEnemy(Aircraft::Raptor,    0.f,  500.f);
	addEnemy(Aircraft::Raptor,    0.f, 1000.f);
	addEnemy(Aircraft::Raptor, +100.f, 1150.f);
	addEnemy(Aircraft::Raptor, -100.f, 1150.f);
	addEnemy(Aircraft::Avenger,  70.f, 1500.f);
	addEnemy(Aircraft::Avenger, -70.f, 1500.f);
	addEnemy(Aircraft::Avenger, -70.f, 1710.f);
	addEnemy(Aircraft::Avenger,  70.f, 1700.f);
	addEnemy(Aircraft::Avenger,  30.f, 1850.f);
	addEnemy(Aircraft::Raptor,  300.f, 2200.f);
	addEnemy(Aircraft::Raptor, -300.f, 2200.f);
	addEnemy(Aircraft::Raptor,    0.f, 2200.f);
	addEnemy(Aircraft::Raptor,    0.f, 2500.f);
	addEnemy(Aircraft::Avenger,-300.f, 2700.f);
	addEnemy(Aircraft::Avenger,-300.f, 2700.f);
	addEnemy(Aircraft::Raptor,    0.f, 3000.f);
	addEnemy(Aircraft::Raptor,  250.f, 3250.f);
	addEnemy(Aircraft::Raptor, -250.f, 3250.f);
	addEnemy(Aircraft::Avenger,   0.f, 3500.f);
	addEnemy(Aircraft::Avenger,   0.f, 3700.f);
	addEnemy(Aircraft::Raptor,    0.f, 3800.f);
	addEnemy(Aircraft::Avenger,   0.f, 4000.f);
	addEnemy(Aircraft::Avenger,-200.f, 4200.f);
	addEnemy(Aircraft::Raptor,  200.f, 4200.f);
	addEnemy(Aircraft::Raptor,    0.f, 4400.f);

	sortEnemies();
}

void World::sortEnemies()
{
	// Sort all enemies according to their y value, such that lower enemies are checked first for spawning
	std::sort(mEnemySpawnPoints.begin(), mEnemySpawnPoints.end(), [] (SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.y < rhs.y;
	});
}

void World::addEnemy(Aircraft::Type type, float relX, float relY)
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
		
		std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.type, mTextures, mFonts));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(180.f);
		if (mNetworkedWorld) enemy->disablePickups();

		mSceneLayers[UpperAir]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();
	}
}

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
	destroyZombies.category = static_cast<int>(Category::EnemyAircraft);
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

void World::guideMissiles()
{
	// Setup command that stores all enemies in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = Category::EnemyAircraft;
	enemyCollector.action = derivedAction<Aircraft>([this] (Aircraft& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveEnemies.push_back(&enemy);
	});

	// Setup command that guides all missiles to the enemy which is currently closest to the player
	Command missileGuider;
	missileGuider.category = Category::AlliedProjectile;
	missileGuider.action = derivedAction<Projectile>([this] (Projectile& missile, sf::Time)
	{
		// Ignore unguided bullets
		if (!missile.isGuided())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Aircraft* closestEnemy = nullptr;

		// Find closest enemy
		FOREACH(Aircraft* enemy, mActiveEnemies)
		{
			float enemyDistance = distance(missile, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			missile.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(missileGuider);
	mActiveEnemies.clear();
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

unsigned int const World::getPlayerTwoScore() const
{
	return mPlayerTwoScore;
}

void World::incrementPlayerTwoScore(unsigned int incBy)
{
	mPlayerTwoScore += incBy;
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

			std::unique_ptr<Aircraft> enemy(new Aircraft(Aircraft::Type::Avenger, mTextures, mFonts));
			enemy->setPosition(xPos, yPos);
			enemy->setRotation(180.f);

			if (shrink(mWorldBoundsBuffer, mWorldBounds).intersects(enemy->getBoundingRect()))
			{
				//Picks A random position outside the view but within world bounds
				int xPos = randomIntExcluding(std::ceil(getViewBounds().left), std::ceil(getViewBounds().width));
				int yPos = randomIntExcluding(std::ceil(getViewBounds().top), std::ceil(getViewBounds().height));

				std::unique_ptr<Aircraft> enemy(new Aircraft(Aircraft::Type::Avenger, mTextures, mFonts));
				enemy->setPosition(xPos, yPos);
				//enemy->setRotation(-mPlayerOneCharacter->getAngle());

				if (shrink(mWorldBoundsBuffer, mWorldBounds).intersects(enemy->getBoundingRect()))
				{
					mSceneLayers[UpperAir]->attachChild(std::move(enemy));
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
void World::guideZombies()
{
	// Setup command that stores all players in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = static_cast<int>(Category::PlayerAircraft);
	enemyCollector.action = derivedAction<Aircraft>([this](Aircraft& player, sf::Time)
	{
		if (!player.isDestroyed())
			mActiveEnemies.push_back(&player);
	});

	// Setup command that guides all grenades to the enemy which is currently closest to the player
	Command zombieGuider;
	zombieGuider.category = static_cast<int>(Category::EnemyAircraft);
	zombieGuider.action = derivedAction<Aircraft>([this](Aircraft& zombie, sf::Time)
	{
		//// Ignore unguided players
		//if (zombie.isPlayer())
		//	return;

		float minDistance = std::numeric_limits<float>::max();
		Aircraft* closestEnemy = nullptr;

		// Find closest enemy
		for (Aircraft* enemy : mActiveEnemies)
		{
			float enemyDistance = distance(zombie, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		//if (closestEnemy)
		//	zombie.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(zombieGuider);
	mActiveEnemies.clear();
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

		//If the list does not contain that rectangle spawn a new object TOFIX
		//if (!containsIntersection(objectRects, boundingRectangle)
		//	&& shrink((mWorldBoundsBuffer * 2), mWorldBounds).contains(sf::Vector2f(xPos, yPos)))
		//{
		//	objectRects.push_back(boundingRectangle);
		//	mSceneLayers[UpperAir]->attachChild(std::move(obstacle));
		//}
	}
}

//Mike
void World::handlePlayerCollision()
{
	// Map bound collision //TOFIX
	//if (!shrink(mWorldBoundsBuffer, mWorldBounds).contains(mPlayerOneCharacter->getPosition()))
	//{
	//	mPlayerOneCharacter->setPosition(mPlayerOneCharacter->getLastPosition());
	//}
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
	auto& character1 = static_cast<Aircraft&>(*pair.first);
	auto& character2 = static_cast<Aircraft&>(*pair.second);

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

	//if ((character1.isZombie() && character2.isZombie())
	//	|| (character1.isPlayer() && character2.isPlayer())) //TOFIX
	//{
	//	//DONE FIX COLLISION
	//}
	//else
	//{
		if (getZombieHitElapsedTime() >= sf::milliseconds(ZOMBIEATTACKDELAY))
		{
			if (!character1.isAllied())
				character2.damage(ZOMBIEATTACKDAMAGE);
			else
				character1.damage(ZOMBIEATTACKDAMAGE);

			resetZombieHitElapsedTime();
		}
	//}
}

//Mike
void World::handleObstacleCollisions(SceneNode::Pair& pair)
{
	auto& character = static_cast<Aircraft&>(*pair.first);
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

	if (projectile.isGuided())
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
			if (Aircraft* zombie = dynamic_cast<Aircraft*>(&entity))
			{
				int currZombiesAlive = getAliveZombieCount();
				setAliveZombieCount(--currZombiesAlive);

				//// Increment Player 1 Score //TOFIX
				//if (projectile.getProjectileId() == 0)
				//{
				//	incrementPlayerOneScore(ZOMBIE_KILL_MULTIPLIER);
				//}
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
	auto& player = static_cast<Aircraft&>(*pair.first);
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

	// Distance between characters and explosions center-points
	//float dV = vectorDistance(entity.getWorldPosition(), explosion.getWorldPosition());

	entity.damage(1);
}

//Mike
World::CollisionType World::GetCollisionType(SceneNode::Pair& pair)
{
	if (matchesCategories(pair, Category::PlayerAircraft, Category::EnemyAircraft)
		|| matchesCategories(pair, Category::EnemyAircraft, Category::EnemyAircraft)
		|| matchesCategories(pair, Category::PlayerAircraft, Category::PlayerAircraft))
	{
		return Character_Character;
	}
	else if (matchesCategories(pair, Category::PlayerAircraft, Category::Pickup))
	{
		return Player_Pickup;
	}
	else if (matchesCategories(pair, Category::PlayerAircraft, Category::Obstacle)
		|| matchesCategories(pair, Category::EnemyAircraft, Category::Obstacle))
	{
		return Character_Obstacle;
	}
	else if (matchesCategories(pair, Category::Obstacle, Category::AlliedProjectile)
		|| matchesCategories(pair, Category::Obstacle, Category::EnemyProjectile))
	{
		return Projectile_Obstacle;
	}
	else if (matchesCategories(pair, Category::EnemyAircraft, Category::AlliedProjectile)
		|| matchesCategories(pair, Category::PlayerAircraft, Category::EnemyProjectile))
	{
		return Projectile_Character;
	}
	else if (matchesCategories(pair, Category::EnemyAircraft, Category::Explosion)
		|| matchesCategories(pair, Category::PlayerAircraft, Category::Explosion))
	{
		return Character_Explosion;
	}
	else { return Default; }
}