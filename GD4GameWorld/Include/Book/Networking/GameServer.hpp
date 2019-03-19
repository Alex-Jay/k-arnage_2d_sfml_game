#ifndef BOOK_GAMESERVER_HPP
#define BOOK_GAMESERVER_HPP

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <vector>
#include <memory>
#include <map>

class GameServer {
public:
	explicit GameServer(sf::Vector2f battlefieldSize);
	~GameServer();

	void onDestroy();


	void notifyPlayerSpawn(sf::Int32 characterIdentifier);
	void notifyPlayerRealtimeChange(sf::Int32 characterIdentifier, sf::Int32 action, bool actionEnabled);
	void notifyPlayerEvent(sf::Int32 characterIdentifier, sf::Int32 action);

	void notifyStartGame();

private:
	// A GameServerRemotePeer refers to one instance of the game, may it be local or from another computer
	struct RemotePeer {
		RemotePeer();

		sf::TcpSocket socket;
		sf::Time lastPacketTime;
		std::vector<sf::Int32> characterIdentifiers;
		bool ready;
		bool timedOut;
	};

	// Structure to store information about current character state
	struct characterInfo {
		sf::Vector2f position;
		sf::Int32 hitpoints;
		sf::Int32 missileAmmo;
		std::map<sf::Int32, bool> realtimeActions;
	};

	// Unique pointer to remote peers
	typedef std::unique_ptr<RemotePeer> PeerPtr;

private:
	void setListening(bool enable);
	void executionThread();
	void tick();
	std::vector<sf::Vector2f> getObjectSpwanPoints(int obstacleCount);
	sf::Time now() const;

	void handleIncomingPackets();
	void handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout);

	void handleIncomingConnections();
	void handleDisconnections();

	void informWorldState(sf::TcpSocket& socket);
	void broadcastMessage(const std::string& message);
	void sendToAll(sf::Packet& packet);
	void updateClientState();

private:
	sf::Thread mThread;
	sf::Clock mClock;
	sf::TcpListener mListenerSocket;
	bool mListeningState;
	sf::Time mClientTimeoutTime;

	std::size_t mMaxConnectedPlayers;
	std::size_t mConnectedPlayers;

	float mWorldHeight;
	sf::FloatRect mBattleFieldRect;
	float mBattleFieldScrollSpeed;

	std::size_t mCharacterCount;
	std::map<sf::Int32, characterInfo> mCharacterInfo;

	std::vector<PeerPtr> mPeers;
	sf::Int32 mCharacterIdentifierCounter;
	bool mWaitingThreadEnd;

	sf::Time mLastSpawnTime;
	sf::Time mTimeForNextSpawn;

	bool obstaclesSpawned{};

	int8_t mZombieCount;
};

#endif // BOOK_GAMESERVER_HPP