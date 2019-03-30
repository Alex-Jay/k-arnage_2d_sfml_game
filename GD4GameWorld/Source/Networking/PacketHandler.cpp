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
		case Server::BroadcastMessage:
			setBroadcastMessage(packet);
			break;

		case Server::JoinLobby:
			joinLobby(packet, false);
			break;

		case Server::SelfJoinLobby:
			joinLobby(packet, true);
			break;

		case Server::PlayerDisconnect:
			handleDisconnect(packet);
			break;

		case Server::LobbyState:
			setInitialLobbyState(packet);
			break;

		case Server::LoadGame:
			mLobby->loadGame();
			break;

		case Server::SetCharacters:
			setPlayers(packet);
			break;

		case Server::SetObstacles:
			setObstacles(packet);
			break;

		case Server::PlayerEvent:
			playerEvent(packet);
			break;

		case Server::PlayerRealtimeChange:
			playerRealTimeChange(packet);
			break;

		case Server::SpawnEnemy:
			spawnZombie(packet);
			break;

		case Server::SpawnPickup:
			spawnPickup(packet);
			break;

		case Server::MissionSuccess:
			mGame->requestStackPush(States::MissionSuccess);
			break;

		case Server::UpdateClientState:
			updateClientState(packet);
			break;

		default:
			break;
	}
}

#pragma endregion

#pragma region Packet Recieving

void PacketHandler::handleDisconnect(sf::Packet& packet)
{
	if (mLobby)
		playerDisconnectLobby(packet);
	else
		playerDisconnectGame(packet);
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
	{
		mLobby->setLocalID(characterIdentifier);
		mLobby->RegisterGameState(characterIdentifier);
	}
		
}

void PacketHandler::playerDisconnectLobby(sf::Packet& packet)
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

void PacketHandler::playerDisconnectGame(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	packet >> characterIdentifier;
	mGame->playerDisconnect(characterIdentifier);
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
	//std::cout << "Recieved SET Obstacoles " << std::endl;
	int16_t obstacleCount;
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

void PacketHandler::playerEvent(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	sf::Int32 action;
	packet >> characterIdentifier >> action;
	mGame->playerEvent(characterIdentifier, action);
}

void PacketHandler::playerRealTimeChange(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	sf::Int32 action;
	bool actionEnabled;
	packet >> characterIdentifier >> action >> actionEnabled;
	mGame->playerRealTimeChange(characterIdentifier, action, actionEnabled);
}

void PacketHandler::spawnZombie(sf::Packet& packet)
{
	int x, y;
	packet >> x >> y;
	mGame->spawnZombie(x,y);
}

void PacketHandler::spawnPickup(sf::Packet& packet)
{
	sf::Int32 type;
	sf::Vector2f position;
	packet >> type >> position.x >> position.y;
	mGame->spawnPickup(type,position);
}

void PacketHandler::updateClientState(sf::Packet& packet)
{
	mGame->oldUpdateClientState(packet);
	//sf::Int32 characterCount;
	//packet >> characterCount;

	//for (sf::Int32 i = 0; i < characterCount; ++i) 
	//{
	//	sf::Vector2f characterPosition;
	//	sf::Int32 characterIdentifier;
	//	packet >> characterIdentifier >> characterPosition.x >> characterPosition.y;

	//	mGame->updateClientState(characterIdentifier, characterPosition);
	//}
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

void PacketHandler::notifyServerReady(sf::TcpSocket* socket, int16_t playerID)
{
	//std::cout << "PLayer " << playerID <<  " NOTIFY SERVER READY: " << std::endl;
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Client::Ready);
	packet << playerID;
	socket->send(packet);
}

void PacketHandler::notifyServerWorldBuilt(sf::TcpSocket* socket)
{
	//std::cout << "NOTIFY SERVER WORLD BUILT " << std::endl;
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Client::WorldBuilt);
	socket->send(packet);
}

void PacketHandler::sendGameEvent(sf::TcpSocket* socket, GameActions::Action gameAction)
{
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Client::GameEvent);
	packet << static_cast<sf::Int32>(gameAction.type);
	packet << gameAction.position.x;
	packet << gameAction.position.y;

	socket->send(packet);
}

#pragma endregion


