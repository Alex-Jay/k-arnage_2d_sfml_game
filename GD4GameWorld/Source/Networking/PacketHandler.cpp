#include "Networking/PacketHandler.hpp"
#include "Networking/NetworkProtocol.hpp"


PacketHandler::PacketHandler() :
	  mClientTimeout(sf::seconds(300.f)) 
	, mTimeSinceLastPacket(sf::seconds(0.f))
	, mConnected(false)
{
}

#pragma region Getters & Setters

void PacketHandler::setLobby(LobbyState* lobby)
{
	mLobby = lobby;
}

void PacketHandler::setGame(MultiplayerGameState* game)
{
	mGame = game;
}

void PacketHandler::setConnected(bool isConnected)
{
	mConnected = isConnected;
}

bool PacketHandler::isConnected()
{
	return mConnected;
}

std::vector<std::string> PacketHandler::getBroadcastMessages()
{
	return mBroadcasts;
}

void PacketHandler::removeBroadcast()
{
	mBroadcasts.erase(mBroadcasts.begin());
}

bool PacketHandler::timedOut()
{
	return mFailedConnectionClock.getElapsedTime() >= sf::seconds(5.f);
}

#pragma endregion

#pragma region Update

bool PacketHandler::update(sf::Time dt, sf::TcpSocket* socket)
{
	sf::Packet packet;

	if (socket->receive(packet) == sf::Socket::Done)
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

#pragma endregion

#pragma region Packet Switch

void PacketHandler::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
	switch (packetType) 
	{
		// Send message to all clients
		case Server::BroadcastMessage: {
			setBroadcastMessage(packet);
		} break;

		case Server::JoinLobby: {
			joinLobby(packet);
		} break;

		case Server::SelfJoinLobby: {
			joinLobby(packet, true);
		} break;

		case Server::PlayerDisconnect: {
			handleDisconnect(packet);
		} break;

		case Server::LobbyState: {
			setInitialLobbyState(packet);
		} break;

		case Server::LoadGame:
		{
			mLobby->loadGame();
		} break;

		case Server::SetCharacters: {
			setPlayers(packet);
		} break;

		case Server::SetObstacles: {
			setObstacles(packet);
		} break;
	}
}

#pragma endregion

#pragma region Packet Recieving

void PacketHandler::handleDisconnect(sf::Packet& packet)
{
	//If lobby is not null it means players are in lobby state
	if (mLobby)
	{
		leaveLobby(packet);
	}
	//else leaveMultiplayer
}

void PacketHandler::setBroadcastMessage(sf::Packet& packet)
{
	std::string message;
	packet >> message;
	mBroadcasts.push_back(message);
}

void PacketHandler::joinLobby(sf::Packet& packet, bool isSelf)
{
	sf::Int32 characterIdentifier;
	packet >> characterIdentifier;
	mLobby->increasePlayerCount();
	mLobby->updateDisplayText();

	if (isSelf)
		mLobby->setLocalID(characterIdentifier);
}

void PacketHandler::leaveLobby(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	packet >> characterIdentifier;

	std::cout << "car ID is " << characterIdentifier << std::endl;
	std::cout << "loby ID is " << mLobby->getLocalID() << std::endl;

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

void PacketHandler::setPlayers(sf::Packet& packet)
{
	sf::Int32 playerCount;
	packet >> playerCount;

	std::vector<sf::Int32> playerIds;
	sf::Int32 localId = mGame->getLocalID();

	for (sf::Int32 i = 0; i < playerCount; ++i)
	{
		sf::Int32 characterIdentifier;
		packet >> characterIdentifier;
		if (characterIdentifier != localId)
		{
			playerIds.push_back(characterIdentifier);
		}
	}

	mGame->spawnPlayers(playerIds);
	mGame->spawnSelf();
}

void PacketHandler::setObstacles(sf::Packet& packet)
{
	sf::Int32 obstacleCount;
	packet >> obstacleCount;
	std::vector<Obstacle::ObstacleData> obstacleData;
	int16_t type, x, y, a;

	for (int i = 0; i < obstacleCount; ++i)
	{
		packet >> type >> x >> y >> a;
		obstacleData.push_back(Obstacle::ObstacleData(type, x, y, a));
	}

	mGame->spawnObstacles(obstacleData);
}


#pragma endregion

#pragma region Packet Sending

void PacketHandler::sendLoadGame(sf::TcpSocket* socket)
{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::LoadGame);
		socket->send(packet);
}

void PacketHandler::sendDisconnectSelf(sf::TcpSocket* socket)
{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		socket->send(packet);
}

void PacketHandler::notifyServerReady(sf::TcpSocket* socket)
{
	std::cout << "NOTIFY SERVER READY: " << std::endl;
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Client::Ready);
	socket->send(packet);
}

void PacketHandler::notifyServerWorldBuilt(sf::TcpSocket* socket)
{
	std::cout << "NOTIFY SERVER WORLD BUILT " << std::endl;
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Client::WorldBuilt);
	socket->send(packet);
}
#pragma endregion


