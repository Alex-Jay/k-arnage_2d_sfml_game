#include "Networking/PacketHandler.hpp"
#include "Networking/NetworkProtocol.hpp"

PacketHandler::PacketHandler() :
	  mClientTimeout(sf::seconds(300.f)) 
	, mTimeSinceLastPacket(sf::seconds(0.f))
	, mConnected(false)
{
}

void PacketHandler::setLobby(LobbyState* lobby)
{
	mLobby = lobby;
}

void PacketHandler::setConnected(bool isConnected)
{
	mConnected = isConnected;
}

bool PacketHandler::isConnected()
{
	return mConnected;
}

bool PacketHandler::timedOut()
{
	return mFailedConnectionClock.getElapsedTime() >= sf::seconds(5.f);
}

bool PacketHandler::update(sf::Time dt, sf::TcpSocket* mSocket)
{
	sf::Packet packet;

	if (mSocket->receive(packet) == sf::Socket::Done) 
	{
		mTimeSinceLastPacket = sf::seconds(0.f);
		sf::Int32 packetType;
		packet >> packetType;
		handlePacket(packetType, packet);
	}
	else if (mTimeSinceLastPacket > mClientTimeout)
	{
		mConnected = false;
		mFailedConnectionClock.restart();
	}

	mTimeSinceLastPacket += dt;

	return true;
}

void PacketHandler::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
	switch (packetType) {
		// Send message to all clients
	case Server::BroadcastMessage: {
		setBroadcastMessage(packet);
	} break;

	case Server::JoinLobby: {
		joinLobby(packet);
	} break;

	case Server::LeaveLobby: {
		leaveLobby(packet);
	} break;

	case Server::LobbyState: {
		setInitialLobbyState(packet);
	} break;

	case Server::LoadGame:
	{
		mLobby->loadGame();
	} break;
	}
}

void PacketHandler::setBroadcastMessage(sf::Packet& packet)
{
	std::string message;
	packet >> message;
	mBroadcasts.push_back(message);
}

void PacketHandler::joinLobby(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	packet >> characterIdentifier;
	mLobby->setLocalID(characterIdentifier);
	mLobby->increasePlayerCount();
	mLobby->updateDisplayText();
}

void PacketHandler::leaveLobby(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	packet >> characterIdentifier;

	if (mLobby->getLocalID() == characterIdentifier)
	{
		mLobby->returnToMenu();
	}
	else
	{
		mLobby->decreasePlayerCount();
		mLobby->updateDisplayText();
	}
}

void PacketHandler::setInitialLobbyState(sf::Packet& packet)
{
	sf::Int32 characterCount;
	packet >> characterCount;
	mLobby->setPlayerCount(characterCount);
	mLobby->updateDisplayText();
}
