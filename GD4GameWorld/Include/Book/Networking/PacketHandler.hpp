#pragma once
#include <SFML/Network/Packet.hpp>

#include"State/LobbyState.hpp"
#include"State/MultiplayerGameState.hpp"

class LobbyState;

class PacketHandler{
public:
	PacketHandler();

	void setLobby(LobbyState * lobby);

	void setConnected(bool isConnected);

	bool isConnected();

	std::vector<std::string> getBroadcastMessages();

	void removeBroadcast();

	bool timedOut();

	bool update(sf::Time dt, sf::TcpSocket* socket);

	void sendLoadGame(sf::TcpSocket * socket);

	void sendDisconnectSelf(sf::TcpSocket * socket);

private:
	void handlePacket(sf::Int32 packetType, sf::Packet & packet);

	void handleDisconnect(sf::Packet & packet);

	void setBroadcastMessage(sf::Packet & packet);

	void joinLobby(sf::Packet & packet, bool isSelf = false);

	void leaveLobby(sf::Packet & packet);

	void setInitialLobbyState(sf::Packet & packet);

private:
	std::vector<std::string> mBroadcasts;
	sf::Time mClientTimeout;
	sf::Time mTimeSinceLastPacket;

	sf::Clock mFailedConnectionClock;

private:
	LobbyState* mLobby;

	bool mConnected;
};
