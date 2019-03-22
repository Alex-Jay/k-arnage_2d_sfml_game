#include "State/MultiplayerGameState.hpp"
#include "Audio/MusicPlayer.hpp"
#include "Structural/Foreach.hpp"
#include "Structural/Utility.hpp"
#include "Constant/Constants.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool isHost)
	: State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds, true)
	, mWindow(*context.window)
	, mSocket(*context.socket)
	, mLocalPlayerID(context.localID)
	, mTextureHolder(*context.textures)
	, mConnected(false)
	, mActiveState(true)
	, mHasFocus(true)
	, mHost(isHost)
	, mGameStarted(false)
	, mClientTimeout(sf::seconds(20.f))
	, mTimeSinceLastPacket(sf::seconds(0.f))
	, mPacketHandler(new PacketHandler)
{

	mPacketHandler->setGame(this);
	mPacketHandler->setConnected(true);

	mBroadcastText.setFont(context.fonts->get(Fonts::Main));
	mBroadcastText.setPosition(1024.f / 2, 100.f);
	mServerNotifiedReady = false;
	
	mLoadingText.setFont(context.fonts->get(Fonts::Main));
	mLoadingText.setCharacterSize(35);
	mLoadingText.setFillColor(sf::Color::White);
	centerOrigin(mLoadingText);
	mLoadingText.setPosition(mWindow.getSize().x / 2.f, mWindow.getSize().y / 2.f);
	mLoadingText.setString("Loading Game...");

	// Play game theme
	context.music->play(Music::MissionTheme);

	mPacketHandler->notifyServerReady(&mSocket);
}

#pragma region Update

void MultiplayerGameState::draw()
{
	if (mGameStarted)
	{
		mWorld.draw();

		// Broadcast messages in default view
		mWindow.setView(mWindow.getDefaultView());

		if (!mBroadcasts.empty())
			mWindow.draw(mBroadcastText);
	}
	else
	{
		mWindow.draw(mLoadingText);
	}

	
}

bool MultiplayerGameState::update(sf::Time dt)
{
	handleCharacterCount(dt);

	handleRealTimeInput();

	handleNetworkInput();

	handleServerMessages(dt);

	updateBroadcastMessage(dt);

	handleGameActions();

	handlePositionUpdates();

	mTimeSinceLastPacket += dt;

	if (!mSeverNotifiedBuilt && mCharactersRecieved && mObstaclesRecieved)
	{
		mPacketHandler->notifyServerWorldBuilt(&mSocket);
		mSeverNotifiedBuilt = true;
		mGameStarted = true;
	}
	return true;
}

void MultiplayerGameState::updateBroadcastMessage(sf::Time elapsedTime)
{
	if (mBroadcasts.empty())
		return;

	// Update broadcast timer
	mBroadcastElapsedTime += elapsedTime;
	if (mBroadcastElapsedTime > sf::seconds(2.5f)) {
		// If message has expired, remove it
		mBroadcasts.erase(mBroadcasts.begin());

		// Continue to display next broadcast message
		if (!mBroadcasts.empty()) {
			mBroadcastText.setString(mBroadcasts.front());
			centerOrigin(mBroadcastText);
			mBroadcastElapsedTime = sf::Time::Zero;
		}
	}
}

#pragma endregion

#pragma region Events



void MultiplayerGameState::disableAllRealtimeActions()
{
	mActiveState = false;

	FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
		mPlayers[identifier]->disableAllRealtimeActions();
}

bool MultiplayerGameState::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();

	// Forward event to all players
	FOREACH(auto& pair, mPlayers)
		pair.second->handleEvent(event, commands);

	if (event.type == sf::Event::KeyPressed)
	{
		// Escape pressed, trigger the pause screen
		if (event.key.code == sf::Keyboard::Escape)
		{
			disableAllRealtimeActions();
			requestStackPush(States::NetworkPause);
		}
	}
	else if (event.type == sf::Event::GainedFocus)
	{
		mHasFocus = true;
	}
	else if (event.type == sf::Event::LostFocus)
	{
		mHasFocus = false;
	}

	return true;
}

void MultiplayerGameState::onActivate()
{
	mActiveState = true;
}

void MultiplayerGameState::onDestroy()
{
	if (!mHost && mConnected) {
		// Inform server this client is dying
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		mSocket.send(packet);
	}
}

#pragma endregion

#pragma region Packet Handling

void MultiplayerGameState::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
	switch (packetType) {
		// Send message to all clients
	//case Server::BroadcastMessage: {
	//	
	//	broadcastMessage(packet);
	//} break;

	//case Server::SpawnSelf: {

	//	std::cout << "SPAWN SELF RECIEVED" << std::endl;
	//	//spawnSelf(packet);
	//} break;

	case Server::SetCharacters: {
		//setCharacters(packet);
	} break;

	case Server::PlayerConnect: {
		//playerConnect(packet);
	} break;

	case Server::PlayerDisconnect: {
		playerDisconnect(packet);
	} break;

	case Server::InitialState: {
		//setInitialState(packet);
	} break;

	case Server::PlayerEvent: {
		playerEvent(packet);
	} break;

	case Server::PlayerRealtimeChange: {
		playerRealTimeChange(packet);
	} break;

	case Server::SpawnEnemy: {
		spawnEnemy(packet);
	} break;

	//case Server::SpawnObstacle: {
	//	spawnObstacle(packet);
	//} break;

	case Server::MissionSuccess: {
		requestStackPush(States::MissionSuccess);
	} break;

	case Server::SpawnPickup: {
		spawnPickup(packet);
	} break;

	case Server::UpdateClientState: {
		updateClientState(packet);
	} break;
	}
}

void MultiplayerGameState::broadcastMessage(sf::Packet& packet)
{
	std::string message;
	packet >> message;
	mBroadcasts.push_back(message);

	// Just added first message, display immediately
	if (mBroadcasts.size() == 1) {
		mBroadcastText.setString(mBroadcasts.front());
		centerOrigin(mBroadcastText);
		mBroadcastElapsedTime = sf::Time::Zero;
	}
}

void MultiplayerGameState::spawnSelf()
{
		Character* character = mWorld.addCharacter(mLocalPlayerID, true);
		//mLocalPlayerIdentifiers.push_back(playerIds[i]);
		character->setPosition(assignCharacterSpawn(mLocalPlayerID));
		mPlayers[mLocalPlayerID].reset(new Player(&mSocket, mLocalPlayerID, getContext().keys));

		mCharactersRecieved = true;
}

void MultiplayerGameState::spawnPlayers(std::vector<sf::Int32> playerIds)
{
	for (size_t i = 0; i < playerIds.size(); ++i)
	{
		Character* character = mWorld.addCharacter(playerIds[i], false);
		//mLocalPlayerIdentifiers.push_back(playerIds[i]);
		character->setPosition(assignCharacterSpawn(playerIds[i]));
		mPlayers[playerIds[i]].reset(new Player(&mSocket, playerIds[i], nullptr));
	}
}

void MultiplayerGameState::spawnObstacles(std::vector<Obstacle::ObstacleData> obstacleData)
{
	for (size_t i = 0; i < obstacleData.size(); i++)
	{
		mWorld.addObstacle(static_cast<Obstacle::ObstacleID>
			(obstacleData[i].type),
			 obstacleData[i].x,
			 obstacleData[i].y,
			 obstacleData[i].a);
	}

	mObstaclesRecieved = true;
}
void MultiplayerGameState::playerDisconnect(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	packet >> characterIdentifier;

	mWorld.removeCharacter(characterIdentifier);
	mPlayers.erase(characterIdentifier);
}


void MultiplayerGameState::playerEvent(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	sf::Int32 action;
	packet >> characterIdentifier >> action;

	auto itr = mPlayers.find(characterIdentifier);
	if (itr != mPlayers.end())
		itr->second->handleNetworkEvent(static_cast<Player::Action>(action), mWorld.getCommandQueue());
}

void MultiplayerGameState::playerRealTimeChange(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	sf::Int32 action;
	bool actionEnabled;
	packet >> characterIdentifier >> action >> actionEnabled;

	auto itr = mPlayers.find(characterIdentifier);
	if (itr != mPlayers.end())
		itr->second->handleNetworkRealtimeChange(static_cast<Player::Action>(action), actionEnabled);
}

void MultiplayerGameState::spawnEnemy(sf::Packet& packet)
{
	int x, y;
	packet >> x >> y;
	mWorld.addZombie(x, y, 0);
}

void MultiplayerGameState::spawnObstacle(sf::Packet& packet)
{
	std::cout << "SPAWN Obstacle RECIEVED" << std::endl;
	
	if (!mObstaclesRecieved)
	{
		float x, y, a;
		sf::Int32 type;
		packet >> type >> x >> y >> a;

		mWorld.addObstacle(static_cast<Obstacle::ObstacleID>(type), x, y, a);
		mObstaclesRecieved = true;
	}

}

void MultiplayerGameState::spawnPickup(sf::Packet& packet)
{
	sf::Int32 type;
	sf::Vector2f position;
	packet >> type >> position.x >> position.y;

	mWorld.createPickup(position, static_cast<Pickup::Type>(type));
}

void MultiplayerGameState::updateClientState(sf::Packet& packet)
{
	float currentWorldPosition;
	sf::Int32 characterCount;
	packet >> currentWorldPosition >> characterCount;

	float currentViewPosition = mWorld.getViewBounds().top + mWorld.getViewBounds().height;

	for (sf::Int32 i = 0; i < characterCount; ++i) {
		sf::Vector2f characterPosition;
		sf::Int32 characterIdentifier;
		packet >> characterIdentifier >> characterPosition.x >> characterPosition.y;

		Character* character = mWorld.getCharacter(characterIdentifier);

		bool isLocalPlane = std::find(mLocalPlayerIdentifiers.begin(), mLocalPlayerIdentifiers.end(), characterIdentifier) != mLocalPlayerIdentifiers.end();
		if (character && !isLocalPlane) {
			sf::Vector2f interpolatedPosition = character->getPosition() + (characterPosition - character->getPosition()) * 0.1f;
			character->setPosition(interpolatedPosition);
		}
	}
}

sf::Vector2f MultiplayerGameState::assignCharacterSpawn(int Identifier)
{
	sf::Vector2f spawnPosition = sf::Vector2f(0, 0);
	switch (Identifier)
	{
		case 0:
			spawnPosition = sf::Vector2f(512.f, 315.f);
			break;
		case 1:
			spawnPosition = sf::Vector2f(712.f, 315.f);
			break;
		case 2:
			spawnPosition = sf::Vector2f(912.f, 315.f);
			break;
		case 3:
			spawnPosition = sf::Vector2f(1112.f, 315.f);
			break;
		case 4:
			spawnPosition = sf::Vector2f(1312.f, 315.f);
			break;
		case 5:
			spawnPosition = sf::Vector2f(1512.f, 315.f);
			break;
		case 6:
			spawnPosition = sf::Vector2f(1712.f, 315.f);
			break;
		case 7:
			spawnPosition = sf::Vector2f(1912.f, 315.f);
			break;
		case 8:
			spawnPosition = sf::Vector2f(2112.f, 315.f);
			break;
		case 9:
			spawnPosition = sf::Vector2f(2312.f, 315.f);
			break;
		default:
			break;
	}

	return spawnPosition;
}

#pragma endregion

#pragma region Handle Updates

void MultiplayerGameState::handleCharacterCount(sf::Time dt)
{
	// Remove players whose characters were destroyed
	bool foundLocalPlayer = false;

	for (auto itr = mPlayers.begin(); itr != mPlayers.end();)
	{
		// Check if there are no more local planes for remote clients
		if (std::find(mLocalPlayerIdentifiers.begin(), mLocalPlayerIdentifiers.end(), itr->first) != mLocalPlayerIdentifiers.end())
			foundLocalPlayer = true;

		if (!mWorld.getCharacter(itr->first))
		{
			itr = mPlayers.erase(itr);

			if (mPlayers.empty())
				requestStackPush(States::GameOver);
		}
		else
			++itr;

		mWorld.update(dt);
	}

	if (!foundLocalPlayer && mGameStarted)
		requestStackPush(States::GameOver);
}

void MultiplayerGameState::handleRealTimeInput()
{
	// Only handle the realtime input if the window has focus and the game is unpaused
	if (mActiveState && mHasFocus) {
		CommandQueue& commands = mWorld.getCommandQueue();

		FOREACH(auto& pair, mPlayers)
			pair.second->handleRealtimeInput(commands);
	}
}

void MultiplayerGameState::handleNetworkInput()
{
	// Always handle the network input
	CommandQueue& commands = mWorld.getCommandQueue();

	FOREACH(auto& pair, mPlayers)
		pair.second->handleRealtimeNetworkInput(commands);
}

void MultiplayerGameState::handleServerMessages(sf::Time dt)
{
	//// Handle messages from server that may have arrived
	//sf::Packet packet;
	//if (mSocket.receive(packet) == sf::Socket::Done) {
	//	mTimeSinceLastPacket = sf::seconds(0.f);
	//	sf::Int32 packetType;
	//	packet >> packetType;
	//	handlePacket(packetType, packet);
	//}
	//else {
	//	// Check for timeout with the server
	//	if (mTimeSinceLastPacket > mClientTimeout) {
	//		mConnected = false;

	//		mFailedConnectionText.setString("Lost connection to server");
	//		centerOrigin(mFailedConnectionText);

	//		mFailedConnectionClock.restart();
	//	}
	//}

			// Connected to server: Handle all the network logic
	if (mPacketHandler->isConnected())
	{
		mPacketHandler->update(dt, &mSocket);
		updateBroadcastMessage(dt);
	}
	else if (mPacketHandler->timedOut()) {
		requestStateClear();
		requestStackPush(States::Menu);
	}
}

void MultiplayerGameState::handleGameActions()
{
	// Events occurring in the game
	GameActions::Action gameAction;
	while (mWorld.pollGameAction(gameAction)) {
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::GameEvent);
		packet << static_cast<sf::Int32>(gameAction.type);
		packet << gameAction.position.x;
		packet << gameAction.position.y;

		mSocket.send(packet);
	}
}

void MultiplayerGameState::handlePositionUpdates()
{
	// Regular position updates
	if (mTickClock.getElapsedTime() > sf::seconds(1.f / 20.f)) {
		sf::Packet positionUpdatePacket;
		positionUpdatePacket << static_cast<sf::Int32>(Client::PositionUpdate);
		positionUpdatePacket << static_cast<sf::Int32>(mLocalPlayerIdentifiers.size());

		FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
		{
			if (Character* character = mWorld.getCharacter(identifier))
				positionUpdatePacket << identifier << character->getPosition().x << character->getPosition().y << static_cast<sf::Int32>(character->getHitpoints()) << static_cast<sf::Int32>(character->getGrenadeAmmo());
		}

		mSocket.send(positionUpdatePacket);
		mTickClock.restart();
	}
}

#pragma endregion


sf::Int32 MultiplayerGameState::getLocalID()
{
	return mLocalPlayerID;
}