#include "Networking/GameServer.hpp"
#include "Networking/NetworkProtocol.hpp"
#include "Structural/Foreach.hpp"
#include "Structural/Utility.hpp"
#include "Entity/Pickup.hpp"
#include "Entity/Character.hpp"
#include "Entity/Obstacle.hpp"
#include "Constant/Constants.hpp"

#include <SFML/Network/Packet.hpp>

#include <iostream>

GameServer::RemotePeer::RemotePeer()
	: ready(false)
	, timedOut(false)
{
	socket.setBlocking(false);
}

GameServer::GameServer(sf::Vector2f battlefieldSize)
	: mThread(&GameServer::executionThread, this)
	, mListeningState(false)
	, mClientTimeoutTime(sf::seconds(300.f))
	, mMaxConnectedPlayers(10)
	, mConnectedPlayers(0)
	, mWorldHeight(5000.f)
	, mBattleFieldRect(0.f, mWorldHeight - battlefieldSize.y, battlefieldSize.x, battlefieldSize.y)
	, mBattleFieldScrollSpeed(0)
	, mCharacterCount(0)
	, mPeers(1)
	, mCharacterIdentifierCounter(0)
	, mWaitingThreadEnd(false)
	, mLastSpawnTime(sf::Time::Zero)
	, mTimeForNextSpawn(sf::seconds(5.f))
	, mZombieCount(0)
	, mClientReadyCount(0)
{
	mListenerSocket.setBlocking(false);
	mPeers[0].reset(new RemotePeer());
	mThread.launch();
}

GameServer::~GameServer()
{
	mWaitingThreadEnd = true;
	mThread.wait();
}

#pragma region Update

void GameServer::setListening(bool enable)
{
	if (!enable)
	{
		mListenerSocket.close();
		mListeningState = false;
	}

	if (!mListeningState)
		mListeningState = (mListenerSocket.listen(ServerPort) == sf::TcpListener::Done);
}

void GameServer::executionThread()
{
	setListening(true);

	sf::Time stepInterval = sf::seconds(1.f / 60.f);
	sf::Time stepTime = sf::Time::Zero;
	sf::Time tickInterval = sf::seconds(1.f / 20.f);
	sf::Time tickTime = sf::Time::Zero;
	sf::Clock stepClock, tickClock;

	while (!mWaitingThreadEnd)
	{
		handleIncomingPackets();
		handleIncomingConnections();

		stepTime += stepClock.getElapsedTime();
		stepClock.restart();

		tickTime += tickClock.getElapsedTime();
		tickClock.restart();

		// Fixed update step
		while (stepTime >= stepInterval)
		{
			stepTime -= stepInterval;
		}

		// Fixed tick step
		while (tickTime >= tickInterval)
		{
			tick();
			tickTime -= tickInterval;
		}

		// Sleep to prevent server from consuming 100% CPU
		sf::sleep(sf::milliseconds(100));
	}
}

void GameServer::tick()
{
	if (mGameStarted)
	{
		updateClientState();
		//TODO HANDLE GAME OVER CONDITION
		//handleWinCondition();
		RemoveDestroyedCharacters();
		spawnEnemys();
	}
	else if(mClientReadyCount >= mPeers.size() - 1)
	{
		//std::cout << "ALL CLIENTS READY: " << std::endl;
		setObstacles();
		sendCharacters();
		mGameStarted = true;
	}
	
}

sf::Time GameServer::now() const
{
	return mClock.getElapsedTime();
}

#pragma endregion

#pragma region Handle Packets

void GameServer::handleIncomingPackets()
{
	bool detectedTimeout = false;

	FOREACH(PeerPtr& peer, mPeers)
	{
		if (peer->ready)
		{
			sf::Packet packet;
			while (peer->socket.receive(packet) == sf::Socket::Done)
			{
				// Interpret packet and react to it
				handleIncomingPacket(packet, *peer, detectedTimeout);

				// Packet was indeed received, update the ping timer
				peer->lastPacketTime = now();
				packet.clear();
			}

			if (now() >= peer->lastPacketTime + mClientTimeoutTime) //TODO INFORM SERVER OF LOBBY STATUS, temp changing mClientTimeoutTime to 5 min
			{
				////std::cout << "4 Setting Timeout status" << std::endl;
				peer->timedOut = true;
				detectedTimeout = true;
			}
		}
	}

	if (detectedTimeout)
		GameServer::handleDisconnections();
}

void GameServer::handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout)
{
	sf::Int32 packetType;
	packet >> packetType;

	////std::cout << "RECIEVED PACKED TYPE: " << packetType << std::endl;

	switch (packetType)
	{
		case Client::Quit:
			receivingPeer.timedOut = true;
			detectedTimeout = true;
			break;
		case Client::LoadGame: /* When Clients are Ready */
			loadGame();
			break;
		case Client::Ready:
			++mClientReadyCount;
			break;
		case Client::PlayerEvent:
			playerEvent(packet);
			break;
		case Client::PlayerRealtimeChange:
			playerRealTimeChange(packet);
			break;
		case Client::PositionUpdate:
			positionUpdate(packet);
			break;
		case Client::GameEvent:
			gameEvent(packet, receivingPeer);
			break;
		default:
			break;
	}
}

#pragma endregion

#pragma region Notify Events

void GameServer::loadGame()
{
	notifyLoadGame();
}

void GameServer::playerEvent(sf::Packet packet)
{
	sf::Int32 characterIdentifier;
	sf::Int32 action;
	packet >> characterIdentifier >> action;
	notifyPlayerEvent(characterIdentifier, action);
}

void GameServer::playerRealTimeChange(sf::Packet packet)
{
	sf::Int32 characterIdentifier;
	sf::Int32 action;
	bool actionEnabled;
	packet >> characterIdentifier >> action >> actionEnabled;
	mCharacterInfo[characterIdentifier].realtimeActions[action] = actionEnabled;
	notifyPlayerRealtimeChange(characterIdentifier, action, actionEnabled);
}

void GameServer::positionUpdate(sf::Packet packet)
{
	//sf::Int32 numCharacters;
	//packet >> numCharacters;

	//for (sf::Int32 i = 0; i < numCharacters; ++i)
	//{
	//	sf::Int32 characterIdentifier;
	//	sf::Int32 characterHitpoints;
	//	sf::Int32 missileAmmo;
	//	sf::Vector2f characterPosition;
	//	packet >> characterIdentifier >> characterPosition.x >> characterPosition.y;
	//	mCharacterInfo[characterIdentifier].position = characterPosition;
	//	//mCharacterInfo[characterIdentifier].hitpoints = characterHitpoints;
	//	//mCharacterInfo[characterIdentifier].missileAmmo = missileAmmo;
	//}

	//OLD CODE
	sf::Int32 characterCount;
	packet >> characterCount;

	for (sf::Int32 i = 0; i < characterCount; ++i)
	{
		sf::Int32 characterIdentifier;
		sf::Int32 characterHitpoints;
		sf::Int32 missileAmmo;
		sf::Vector2f characterPosition;
		packet >> characterIdentifier >> characterPosition.x >> characterPosition.y >> characterHitpoints >> missileAmmo;
		mCharacterInfo[characterIdentifier].position = characterPosition;
		mCharacterInfo[characterIdentifier].hitpoints = characterHitpoints;
		mCharacterInfo[characterIdentifier].missileAmmo = missileAmmo;
	}
}

void GameServer::gameEvent(sf::Packet packet, RemotePeer& receivingPeer)
{
	sf::Int32 action;
	float x, y;

	packet >> action;
	packet >> x;
	packet >> y;

	// Enemy explodes: With certain probability, drop pickup
	// To avoid multiple messages spawning multiple pickups, only listen to first peer (host)
	if (action == GameActions::EnemyExplode && randomInt(3) == 0 && &receivingPeer == mPeers[0].get())
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::SpawnPickup);
		packet << static_cast<sf::Int32>(randomInt(static_cast<sf::Int32>(Pickup::Type::TypeCount)));
		packet << x;
		packet << y;

		sendToAll(packet);
	}

	if (action == GameActions::EnemyExplode)
		mZombieCount--;
}

#pragma endregion

#pragma region Notify Clients

void GameServer::notifyPlayerRealtimeChange(sf::Int32 characterIdentifier, sf::Int32 action, bool actionEnabled)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerRealtimeChange);
			packet << characterIdentifier;
			packet << action;
			packet << actionEnabled;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerEvent(sf::Int32 characterIdentifier, sf::Int32 action)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerEvent);
			packet << characterIdentifier;
			packet << action;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyLoadGame()
{
	//std::cout << "NOTIFYING LOAD GAME: " << std::endl;
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::LoadGame);
			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyStartGame()
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::StartGame);
			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerJoin(sf::Int32 characterIdentifier)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::JoinLobby);
			packet << characterIdentifier;;
			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::updateClientState()
{
	//sf::Packet updateClientStatePacket;
	//updateClientStatePacket << static_cast<sf::Int32>(Server::UpdateClientState);
	////updateClientStatePacket << static_cast<float>(mBattleFieldRect.top + mBattleFieldRect.height);
	//updateClientStatePacket << static_cast<sf::Int32>(mCharacterInfo.size());

	//FOREACH(auto character, mCharacterInfo)
	//	updateClientStatePacket << character.first << character.second.position.x << character.second.position.y;

	//sendToAll(updateClientStatePacket);

	//OLD CODE
	sf::Packet updateClientStatePacket;
	updateClientStatePacket << static_cast<sf::Int32>(Server::UpdateClientState);
	updateClientStatePacket << static_cast<float>(mBattleFieldRect.top + mBattleFieldRect.height);
	updateClientStatePacket << static_cast<sf::Int32>(mCharacterInfo.size());

	FOREACH(auto character, mCharacterInfo)
	{
		updateClientStatePacket << character.first << character.second.position.x << character.second.position.y;
		//std::cout << "RECEIVED POS: " << character.second.position.x << ", " << character.second.position.y << std::endl;
	}

	sendToAll(updateClientStatePacket);
}

// Tell the newly connected peer about how the world is currently
void GameServer::informWorldState(sf::TcpSocket& socket)
{
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Server::LobbyState);

	packet << static_cast<sf::Int32>(mCharacterCount);

	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			FOREACH(sf::Int32 identifier, mPeers[i]->characterIdentifiers)
				packet << identifier;
		}
	}

	socket.send(packet);
}

void GameServer::broadcastMessage(const std::string& message)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::BroadcastMessage);
			packet << message;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::sendToAll(sf::Packet& packet)
{
	FOREACH(PeerPtr& peer, mPeers)
	{
		if (peer->ready)
			peer->socket.send(packet);
	}
}
#pragma endregion

#pragma region Connection Logic

void GameServer::handleIncomingConnections()
{
	if (!mListeningState)
		return;

	if (mListenerSocket.accept(mPeers[mConnectedPlayers]->socket) == sf::TcpListener::Done)
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::SelfJoinLobby);
		packet << mCharacterIdentifierCounter;

		mPeers[mConnectedPlayers]->characterIdentifiers.push_back(mCharacterIdentifierCounter);

		broadcastMessage("New player!");
		
		notifyPlayerJoin(mCharacterIdentifierCounter++);

		informWorldState(mPeers[mConnectedPlayers]->socket);

		mPeers[mConnectedPlayers]->socket.send(packet);
		mPeers[mConnectedPlayers]->ready = true;
		mPeers[mConnectedPlayers]->lastPacketTime = now(); // prevent initial timeouts
		mCharacterCount++;
		mConnectedPlayers++;

		//std::cout << "PLAYER ADDED " << std::endl;

		//std::cout << "CHARC COUNT " << mCharacterCount << std::endl;

		//std::cout << "PLAYER CONNECTED " << mConnectedPlayers << std::endl;

		//std::cout << "PEERS" << mPeers.size() << std::endl;
		if (mConnectedPlayers >= mMaxConnectedPlayers)
			setListening(false);
		else // Add a new waiting peer
			mPeers.push_back(PeerPtr(new RemotePeer()));
	}
}

void GameServer::handleDisconnections()
{
	for (auto itr = mPeers.begin(); itr != mPeers.end(); )
	{
		if ((*itr)->timedOut)
		{
			// Inform everyone of the disconnection, erase 
			FOREACH(sf::Int32 identifier, (*itr)->characterIdentifiers)
			{
				sendToAll(sf::Packet() << static_cast<sf::Int32>(Server::PlayerDisconnect) << identifier);

				mCharacterInfo.erase(identifier);
			}

			mConnectedPlayers--;
			mCharacterCount -= (*itr)->characterIdentifiers.size();

			itr = mPeers.erase(itr);

			// Go back to a listening state if needed
			if (mConnectedPlayers < mMaxConnectedPlayers)
			{
				mPeers.push_back(PeerPtr(new RemotePeer()));
				setListening(true);
			}

			broadcastMessage("A Player has disconnected.");
		}
		else
		{
			++itr;
		}
	}
}

#pragma endregion

#pragma region World Logic

void GameServer::RemoveDestroyedCharacters()
{
	// Remove IDs of character that have been destroyed (relevant if a client has two, and loses one)
	for (auto itr = mCharacterInfo.begin(); itr != mCharacterInfo.end(); )
	{
		if (itr->second.hitpoints <= 0)
			mCharacterInfo.erase(itr++);
		else
			++itr;
	}
}

void GameServer::sendCharacters()
{
	//std::cout << "SENDING SET CHARACTERS " << std::endl;

	sf::Packet packet;

	packet << static_cast<sf::Int32>(Server::SetCharacters);
	packet << static_cast<sf::Int32>(mCharacterCount);

	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			FOREACH(sf::Int32 identifier, mPeers[i]->characterIdentifiers)
				packet << identifier;
		}
	}

	sendToAll(packet);
}

void GameServer::setObstacles()
{
	if (!obstaclesSpawned)
	{
		//std::cout << "SENDING SET Obstacoles " << std::endl;

		int16_t obstacleCount = 5; //1u + randomInt(20);

		std::vector<Obstacle::ObstacleData> obstacleData = GameServer::getObjectData(obstacleCount);

		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::SetObstacles);
		packet << obstacleCount;

		// Send the spawn orders to all clients
		for (std::size_t i = 0; i < obstacleCount; ++i)
		{
			packet << obstacleData[i].type;
			packet << obstacleData[i].x;
			packet << obstacleData[i].y;
			packet << obstacleData[i].a;
		}

		sendToAll(packet);

		obstaclesSpawned = true;
	}
}

void GameServer::spawnEnemys()
{
	// Check if its time to attempt to spawn enemies
	if (now() >= mTimeForNextSpawn + mLastSpawnTime && (mZombieCount < MAX_ALIVE_ZOMBIES))
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::SpawnEnemy);
		packet << randomIntExcluding(0, WORLD_WIDTH);
		packet << randomIntExcluding(0, WORLD_HEIGHT);

		sendToAll(packet);

		++mZombieCount;

		mLastSpawnTime = now();
		mTimeForNextSpawn = sf::milliseconds(MIN_ZOMBIE_SPAWN_TIME + randomInt(MAX_ZOMBIE_SPAWN_TIME));
	}
}

std::vector<sf::Vector2f> GameServer::getObjectSpwanPoints(int obstacleCount)
{
	std::vector<sf::Vector2f> spawnPoints;
	std::list<sf::FloatRect> objectRects;
	int xPos, yPos;

	sf::FloatRect worldBounds = sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT));

	for (size_t i = 0; i < obstacleCount; ++i)
	{
		xPos = DESSERT_TILE_WIDTH + randomInt(std::ceil(WORLD_WIDTH - DESSERT_TILE_WIDTH));
		yPos = DESSERT_TILE_HEIGHT + randomInt(std::ceil(WORLD_HEIGHT - DESSERT_TILE_HEIGHT));
		sf::FloatRect boundingRectangle = sf::FloatRect(xPos, yPos, DESSERT_TILE_WIDTH * 2, DESSERT_TILE_HEIGHT * 2);

		if (!containsIntersection(objectRects, boundingRectangle))
		{
			objectRects.push_back(boundingRectangle);
			spawnPoints.push_back(sf::Vector2f(xPos, yPos));
		}
		else
			i--;
	}

	return spawnPoints;
}

std::vector<Obstacle::ObstacleData> GameServer::getObjectData(int obstacleCount)
{
	std::vector<Obstacle::ObstacleData> obstacleData;
	std::list<sf::FloatRect> objectRects;
	int xPos, yPos;

	sf::FloatRect worldBounds = sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT));

	for (size_t i = 0; i < obstacleCount; ++i)
	{
		xPos = DESSERT_TILE_WIDTH + randomInt(std::ceil(WORLD_WIDTH - DESSERT_TILE_WIDTH));
		yPos = DESSERT_TILE_HEIGHT + randomInt(std::ceil(WORLD_HEIGHT - DESSERT_TILE_HEIGHT));
		sf::FloatRect boundingRectangle = sf::FloatRect(xPos, yPos, DESSERT_TILE_WIDTH * 2, DESSERT_TILE_HEIGHT * 2);

		if (!containsIntersection(objectRects, boundingRectangle))
		{
			objectRects.push_back(boundingRectangle);

			int16_t type = randomInt(static_cast<int16_t>(Obstacle::ObstacleID::TypeCount) - 1);
			int16_t rot = randomInt(360);

			obstacleData.push_back(Obstacle::ObstacleData(type, xPos, yPos, rot));
		}
		else
			i--;
	}

	return obstacleData;
}

#pragma endregion
