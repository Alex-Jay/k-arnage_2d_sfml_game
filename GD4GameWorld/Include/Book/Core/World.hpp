#pragma once
#include "Structural/ResourceHolder.hpp"
#include "Node/SceneNode.hpp"
#include "Command/CommandQueue.hpp"
#include "Command/Command.hpp"
#include "Entity/Pickup.hpp"
#include "Effect/BloomEffect.hpp"
#include "Audio/SoundPlayer.hpp"
#include "Networking/NetworkProtocol.hpp"

#include "Structural/ResourceIdentifiers.hpp"
#include "Node/SpriteNode.hpp"
#include "Structural/MapTiler.hpp"
#include "Effect/DistortionEffect.hpp"
#include "Entity/Character.hpp"
#include "Entity/Obstacle.hpp"
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <queue>

//Forward declaration
namespace sf {
	class RenderTarget;
}

class NetworkNode;

class World : private sf::NonCopyable {
private:
	enum CollisionType {
		Character_Character,
		Player_Pickup,
		Character_Obstacle,
		Projectile_Obstacle,
		Projectile_Character,
		Character_Explosion,
		Default
	};

public:
	explicit World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked = false);

	void update(sf::Time dt);
	void draw();

	sf::Time getZombieHitDelay();
	sf::Time getZombieHitElapsedTime();
	sf::Vector3f getMainfold(const sf::FloatRect& overlap, const sf::Vector2f& collisionNormal);
	sf::FloatRect getViewBounds() const;

	CommandQueue& getCommandQueue();

	Character* getCharacter(sf::Int32 localIdentifier) const;
	Character* addCharacter(int8_t identifier, bool isLocal);

	bool pollGameAction(GameActions::Action& out);
	bool hasAlivePlayer() const;

	int8_t getAliveZombieCount();

	void incrementZombieHitElapsedTime(sf::Time dt);
	void resetZombieHitElapsedTime();
	void removeCharacter(int8_t localIdentifier);
	void createPickup(sf::Vector2f position, Pickup::Type type);
	void addObstacle(Obstacle::ObstacleID type, int16_t x, int16_t y, int16_t a);
	void spawnZombies();
	void spawnObstacles();
	void setZombieHitDelay(sf::Time delay);
	void setAliveZombieCount(int8_t count);
	void addZombie(int16_t x, int16_t y);
private:
	struct SpawnPoint {
		SpawnPoint(Character::Type type, int16_t x, int16_t y, int8_t a)
			: type(type)
			, x(x)
			, y(y)
			, a(a)
		{
		}

		Character::Type type;
		int16_t x;
		int16_t y;
		int8_t a;
	};

	struct obstacleSpawnPoint {
		obstacleSpawnPoint(Obstacle::ObstacleID type, int16_t x, int16_t y, int8_t a)
			: type(type)
			, x(x)
			, y(y)
			, a(a)
		{
		}

		Obstacle::ObstacleID type;
		int16_t x;
		int16_t y;
		int8_t a;
	};

	enum Layer {
		Background,
		LowerLayer,
		UpperLayer,
		LayerCount
	};

	CollisionType GetCollisionType(SceneNode::Pair& pair);

	sf::FloatRect getBattlefieldBounds() const;

	void loadTextures();
	void buildScene();
	void addObstacles();
	void handlePlayerCollision();
	void handleCollisions(sf::Time dt);
	void handleCharacterCollisions(SceneNode::Pair& pair);
	void handleObstacleCollisions(SceneNode::Pair& pair);
	void handleProjectileCollisions(SceneNode::Pair& pair);
	void handlePickupCollisions(SceneNode::Pair& pair);
	void handleExplosionCollisions(SceneNode::Pair& pair);
	void setView();
	void addZombies(sf::Time dt);
	
	void updateSounds();
	void StartZombieSpawnTimer(sf::Time dt);
	void destroyEntitiesOutsideView();
	void guideZombies();

private:
	std::array<SceneNode*, static_cast<int>(Layer::LayerCount)> mSceneLayers;
	std::vector<Character*> mPlayerCharacters;
	std::vector<SpawnPoint> mEnemySpawnPoints;
	std::vector<obstacleSpawnPoint> mObstacleSpawnPoints;
	std::vector<Character*> mActiveEnemies;

	sf::RenderTarget& mTarget;
	sf::RenderTexture mSceneTexture;
	sf::RenderTexture mWaterSceneTexture;
	sf::View mWorldView;
	sf::FloatRect mWorldBounds;
	sf::Time mZombieSpawnTimer;
	sf::Time mZombieHitDelay;
	sf::Time mZombieHitElapsedTime;

	TextureHolder mTextures;
	FontHolder& mFonts;
	SoundPlayer& mSounds;
	TextNode* mScoreText;
	SceneNode mSceneGraph;
	CommandQueue mCommandQueue;
	DistortionEffect mDistortionEffect;
	BloomEffect mBloomEffect;
	NetworkNode* mNetworkNode;

	bool mZombieSpawnTimerStarted{};

	sf::Texture mWaterTexture;
	SpriteNode mWaterSprite;

	bool mNetworkedWorld;

	int16_t mWorldBoundsBuffer;
	int16_t mZombieSpawnTime;
	int16_t mNumZombiesSpawn;
	int16_t mNumZombiesAlive;
	sf::Int32 localCharacterID;
};
