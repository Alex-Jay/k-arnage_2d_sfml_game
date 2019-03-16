#ifndef BOOK_WORLD_HPP
#define BOOK_WORLD_HPP

#include "Structural/ResourceHolder.hpp"
#include "Node/SceneNode.hpp"
#include "Entity/Aircraft.hpp"
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

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <queue>


// Forward declaration
namespace sf
{
	class RenderTarget;
}

class NetworkNode;

class World : private sf::NonCopyable
{
private:
	enum CollisionType
	{
		Character_Character,
		Player_Pickup,
		Character_Obstacle,
		Projectile_Obstacle,
		Projectile_Character,
		Character_Explosion,
		Default
	};

	public:
											World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked = false);
		void								update(sf::Time dt);
		void								draw();

		sf::FloatRect						getViewBounds() const;		
		CommandQueue&						getCommandQueue();
		Aircraft*							addAircraft(int identifier);
		void								removeAircraft(int identifier);
		void								setCurrentBattleFieldPosition(float lineY);
		void								setWorldHeight(float height);

		void								addEnemy(Aircraft::Type type, float relX, float relY);
		void								sortEnemies();

		bool 								hasAlivePlayer() const;
		bool 								hasPlayerReachedEnd() const;

		void								setWorldScrollCompensation(float compensation);

		Aircraft*							getAircraft(int identifier) const;
		sf::FloatRect						getBattlefieldBounds() const;

		void								createPickup(sf::Vector2f position, Pickup::Type type);
		bool								pollGameAction(GameActions::Action& out);

		sf::Time getZombieHitDelay();
		void setZombieHitDelay(sf::Time delay);

		sf::Time getZombieHitElapsedTime();
		void incrementZombieHitElapsedTime(sf::Time dt);
		void resetZombieHitElapsedTime();
		sf::Vector3f getMainfold(const sf::FloatRect& overlap, const sf::Vector2f& collisionNormal);
		int getAliveZombieCount();
		void setAliveZombieCount(int count);

		unsigned int const getPlayerOneScore() const;
		void incrementPlayerOneScore(unsigned int incBy);
		unsigned int const getPlayerTwoScore() const;
		void incrementPlayerTwoScore(unsigned int incBy);

	private:
		void								loadTextures();
		void								adaptPlayerPosition();
		void								adaptPlayerVelocity();
		void								updateSounds();

		void								buildScene();
		void								addEnemies();
		void								spawnEnemies();
		void								destroyEntitiesOutsideView();
		void								guideMissiles();

		void SpawnObstacles();
		void handlePlayerCollision();
		void handleCollisions(sf::Time dt);
		void handleCharacterCollisions(SceneNode::Pair& pair);
		void handleObstacleCollisions(SceneNode::Pair& pair);
		void handleProjectileCollisions(SceneNode::Pair& pair);
		void handlePickupCollisions(SceneNode::Pair& pair);
		void handleExplosionCollisions(SceneNode::Pair& pair);
		CollisionType GetCollisionType(SceneNode::Pair& pair);

		void spawnZombies(sf::Time dt);

		void StartZombieSpawnTimer(sf::Time dt);
		void guideZombies();

	private:
		enum Layer
		{
			Background,
			LowerAir,
			UpperAir,
			LayerCount
		};

		struct SpawnPoint 
		{
			SpawnPoint(Aircraft::Type type, float x, float y)
			: type(type)
			, x(x)
			, y(y)
			{
			}

			Aircraft::Type type;
			float x;
			float y;
		};


	private:
		sf::RenderTarget&					mTarget;
		sf::RenderTexture					mSceneTexture;
		sf::View							mWorldView;
		TextureHolder						mTextures;
		FontHolder&							mFonts;
		SoundPlayer&						mSounds;

		SceneNode							mSceneGraph;
		std::array<SceneNode*, LayerCount>	mSceneLayers;
		CommandQueue						mCommandQueue;

		sf::FloatRect						mWorldBounds;
		sf::Vector2f						mSpawnPosition;
		float								mScrollSpeed;
		float								mScrollSpeedCompensation;
		std::vector<Aircraft*>				mPlayerAircrafts;

		std::vector<SpawnPoint>				mEnemySpawnPoints;
		std::vector<Aircraft*>				mActiveEnemies;

		BloomEffect							mBloomEffect;

		bool								mNetworkedWorld;
		NetworkNode*						mNetworkNode;
		SpriteNode*							mFinishSprite;

		//sf::RenderTexture mWaterSceneTexture;
		//TextNode* mScoreText;
		//DistortionEffect mDistortionEffect;

		//Character* mPlayerOneCharacter;
		//Character* mPlayerTwoCharacter;
		//std::vector<Character*> mPlayerCharacters;

		bool mZombieSpawnTimerStarted{};

		sf::Time mZombieSpawnTimer;
		sf::Time mZombieHitDelay;
		sf::Time mZombieHitElapsedTime;

		//sf::Texture mWaterTexture;
		//SpriteNode mWaterSprite;

		int mWorldBoundsBuffer;
		int mZombieSpawnTime;
		int mNumZombiesSpawn;
		int mNumZombiesAlive;

		unsigned int mPlayerOneScore;
		unsigned int mPlayerTwoScore;
};

#endif // BOOK_WORLD_HPP
