#ifndef BOOK_MULTIPLAYERGAMESTATE_HPP
#define BOOK_MULTIPLAYERGAMESTATE_HPP

#include "State/State.hpp"
#include "Core/World.hpp"
#include "Entity/Player.hpp"
#include "Entity/Obstacle.hpp"
#include "Networking/GameServer.hpp"
#include "Networking/NetworkProtocol.hpp"
#include "Networking/PacketHandler.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/Packet.hpp>

class PacketHandler;

class MultiplayerGameState : public State
{
public:
	MultiplayerGameState(StateStack & stack, Context context, bool isHost, sf::Int32 localID);

	virtual void				draw();
	virtual bool				update(sf::Time dt);
	virtual bool				handleEvent(const sf::Event& event);
	virtual void				onActivate();
	void						onDestroy();


	void						disableAllRealtimeActions();
	sf::Int32					getLocalID();

	void						spawnSelf();

	void						spawnPlayers(std::vector<sf::Int32> playerIds);
	void						spawnObstacles(std::vector<Obstacle::ObstacleData> obstacleData);
	void						playerDisconnect(sf::Int32 characterIdentifier);
	void						playerEvent(sf::Int32 characterIdentifier, sf::Int32 action);
	void						playerRealTimeChange(sf::Int32 characterIdentifier, sf::Int32 action, bool actionEnabled);
	void						spawnZombie(int x, int y);
	void						spawnPickup(sf::Int32 type, sf::Vector2f position);
	void						updateClientState(sf::Int32 characterIdentifier, sf::Vector2f characterPosition);
	void						oldUpdateClientState(sf::Packet packet);
private:
	void						updateBroadcastMessage(sf::Time elapsedTime);

	void						broadcastMessage(sf::Packet & packet);


	sf::Vector2f				assignCharacterSpawn(sf::Int32 Identifier);

	void						handleCharacterCount(sf::Time dt);

	void						handleRealTimeInput();

	void						handleNetworkInput();

	void						handleServerMessages(sf::Time dt);

	void						handleGameActions();

	void						handlePositionUpdates();


private:
	typedef std::unique_ptr<Player> PlayerPtr;


private:
	World						mWorld;
	sf::RenderWindow&			mWindow;
	sf::TcpSocket&				mSocket;
	uint16_t					mLocalPlayerID;
	TextureHolder&				mTextureHolder;

	std::map<int, PlayerPtr>	mPlayers;
	std::vector<sf::Int32>		mLocalPlayerIdentifiers;
	bool						mConnected;
	std::unique_ptr<GameServer> mGameServer;
	sf::Clock					mTickClock;

	std::vector<std::string>	mBroadcasts;
	sf::Text					mBroadcastText;
	sf::Time					mBroadcastElapsedTime;

	sf::Text					mLoadingText;

	sf::Text					mFailedConnectionText;
	sf::Clock					mFailedConnectionClock;

	bool						mActiveState;
	bool						mHasFocus;
	bool						mHost;
	bool						mGameStarted;
	bool						mSpectating;
	bool						mCharactersRecieved{};
	bool						mObstaclesRecieved{};
	bool						mServerNotifiedReady{};
	bool						mSeverNotifiedBuilt{};
	sf::Time					mClientTimeout;
	sf::Time					mTimeSinceLastPacket;

	PacketHandler*				mPacketHandler;
};

#endif // BOOK_MULTIPLAYERGAMESTATE_HPP