#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "Character.hpp"
#include "Command.hpp"
#include "CommandQueue.hpp"
#include "BloomEffect.hpp"
#include "SoundPlayer.hpp"

#include "SFML/System/NonCopyable.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Graphics/Texture.hpp"

#include <array>
#include <queue>
#include "MapTiler.hpp"

//Forward declaration
namespace sf
{
	class RenderTarget;
}

class World : private sf::NonCopyable {
private:
	enum CollisionType { Character_Character, Player_Pickup, Player_Obstacle, Projectile_Obstacle, Character_Projectile, Character_Explosion, Default };
public:
	
	explicit World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds);
	void update(sf::Time dt);
	void draw();

	CommandQueue& getCommandQueue();

	bool hasAlivePlayer() const;
	bool hasPlayerReachedEnd() const;

private:
	void loadTextures();
	void buildScene();
	void adaptPlayerPosition();
	void adaptPlayerVelocity();
	void handlePlayerCollision();
	void handleCollisions();
	void handleCharacterCollisions(SceneNode::Pair & colliders);
	void handleObstacleCollisions(SceneNode::Pair & colliders);
	void handleProjectileCollisions(SceneNode::Pair & colliders);
	void handlePickupCollisions(SceneNode::Pair & colliders);
	void handleExplosionCollisions(SceneNode::Pair & pair);
	CollisionType GetCollisionType(SceneNode::Pair & pair);
	void updateSounds();

	void addEnemies();
	void addEnemy(Character::Type type, float relX, float relY);
	void spawnEnemies();
	void destroyEntitiesOutsideView();
	void guideZombies();
	sf::FloatRect getViewBounds() const;
	sf::FloatRect getBattlefieldBounds() const;

	void createObstacle(SceneNode & node, const TextureHolder & textures, sf::Vector2f position) const;

private:
	//TODO REMOVE ALL TRACES OF AIR AND AIRCRAFT ETC...
	enum Layer { Background, LowerAir, UpperAir, LayerCount };
	
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

	CollisionType GetCollisionType(unsigned int collider);
private:
	sf::RenderTarget& mTarget;
	sf::RenderTexture mSceneTexture;
	sf::View mWorldView;
	TextureHolder mTextures;
	FontHolder&	mFonts;
	SoundPlayer& mSounds;

	//MapTiler& map;

	SceneNode mSceneGraph;
	std::array<SceneNode*, static_cast<int>(Layer::LayerCount)> mSceneLayers;
	CommandQueue mCommandQueue;

	sf::FloatRect mWorldBounds;
	sf::Vector2f mSpawnPosition;
	float mScrollSpeed;
	Character* mPlayerCharacter;

	std::vector<SpawnPoint>	mEnemySpawnPoints;
	std::vector<Character*> mActiveEnemies;

	BloomEffect	mBloomEffect;
};