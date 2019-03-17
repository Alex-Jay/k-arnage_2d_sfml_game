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

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <queue>

//Forward declaration
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

			explicit World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked = false);
			void update(sf::Time dt);
			void draw();

			sf::Time getZombieHitDelay();
			void setZombieHitDelay(sf::Time delay);

			sf::Time getZombieHitElapsedTime();
			void incrementZombieHitElapsedTime(sf::Time dt);
			void resetZombieHitElapsedTime();

			CommandQueue& getCommandQueue();

			bool hasAlivePlayer() const;
			sf::Vector3f getMainfold(const sf::FloatRect& overlap, const sf::Vector2f& collisionNormal);
			int getAliveZombieCount();
			void setAliveZombieCount(int count);

			Character* getCharacter(int localIdentifier) const;
			void removeCharacter(int localIdentifier);
			Character * addCharacter(int identifier, bool isLocal);


			bool pollGameAction(GameActions::Action & out);

			void createPickup(sf::Vector2f position, Pickup::Type type);

			

			void addEnemy(Character::Type type, float relX, float relY);

			void sortEnemies();

			sf::FloatRect getViewBounds() const;
		private:
			void loadTextures();
			void buildScene();
			void SpawnObstacles();
			void adaptPlayerPosition();
			void adaptPlayerVelocity();
			void handlePlayerCollision();
			void handleCollisions(sf::Time dt);
			void handleCharacterCollisions(SceneNode::Pair& pair);
			void handleObstacleCollisions(SceneNode::Pair& pair);
			void handleProjectileCollisions(SceneNode::Pair& pair);
			void handlePickupCollisions(SceneNode::Pair& pair);
			void handleExplosionCollisions(SceneNode::Pair& pair);
			CollisionType GetCollisionType(SceneNode::Pair& pair);
			
			void updateSounds();

			void spawnZombies(sf::Time dt);

			void StartZombieSpawnTimer(sf::Time dt);

			void destroyEntitiesOutsideView();
			void guideZombies();
			
			sf::FloatRect getBattlefieldBounds() const;

		private:
			enum Layer { Background, LowerLayer, UpperLayer, LayerCount };

			struct SpawnPoint
			{
				SpawnPoint(Character::Type type, float x, float y)
					: type(type)
					, x(x)
					, y(y)
				{
				}

				Character::Type type;
				float x;
				float y;
			};

		private:
			sf::RenderTarget& mTarget;
			sf::RenderTexture mSceneTexture;
			sf::RenderTexture mWaterSceneTexture;
			sf::View mWorldView;
			TextureHolder mTextures;
			FontHolder& mFonts;
			SoundPlayer& mSounds;

			TextNode* mScoreText;

			SceneNode mSceneGraph;
			//SceneNode mWaterGraph;

			std::array<SceneNode*, static_cast<int>(Layer::LayerCount)> mSceneLayers;
			//std::array<SceneNode*, static_cast<int>(Layer::LayerCount)> mWaterLayers;
			CommandQueue mCommandQueue;

			sf::FloatRect mWorldBounds;
			sf::Vector2f mSpawnPosition;
			float mScrollSpeed;

			//Character* mPlayerLocalCharacter;

			std::vector<Character*> mPlayerCharacters;

			std::vector<SpawnPoint> mEnemySpawnPoints;
			std::vector<Character*> mActiveEnemies;

			DistortionEffect mDistortionEffect;
			BloomEffect mBloomEffect;

			bool mZombieSpawnTimerStarted{};

			sf::Time mZombieSpawnTimer;
			sf::Time mZombieHitDelay;
			sf::Time mZombieHitElapsedTime;

			sf::Texture mWaterTexture;
			SpriteNode mWaterSprite;

			int mWorldBoundsBuffer;
			int mZombieSpawnTime;
			int mNumZombiesSpawn;
			int mNumZombiesAlive;

			int localCharacterID;

			bool								mNetworkedWorld;
			NetworkNode*						mNetworkNode;
		};
