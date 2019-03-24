#include "State/MultiplayerGameState.hpp"
#include "Audio/MusicPlayer.hpp"
#include "Structural/Foreach.hpp"
#include "Structural/Utility.hpp"
#include "Constant/Constants.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool isHost, sf::Int32 localID)
	: State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds, true)
	, mWindow(*context.window)
	, mSocket(*context.socket)
	, mTextureHolder(*context.textures)
	, mConnected(false)
	, mActiveState(true)
	, mHasFocus(true)
	, mHost(isHost)
	, mGameStarted(false)
	, mClientTimeout(sf::seconds(20.f))
	, mTimeSinceLastPacket(sf::seconds(0.f))
	, mPacketHandler(new PacketHandler)
	, mLocalPlayerID(localID)
{
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

	std::cout << "MULTIPLATER ID: " << mLocalPlayerID << std::endl;
	std::cout << "SOCKET Port: " << mSocket.getLocalPort() << std::endl;
	std::cout << "SOCKET r address: " << mSocket.getRemoteAddress() << std::endl;
	std::cout << "SOCKET r Port: " << mSocket.getRemotePort() << std::endl;

	mPacketHandler->setGame(this);
	mPacketHandler->setConnected(true);
	mPacketHandler->notifyServerReady(&mSocket, mLocalPlayerID);
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
	if (mPacketHandler->getBroadcastMessages().empty())
		return;

	// Update broadcast timer
	mBroadcastElapsedTime += elapsedTime;
	if (mBroadcastElapsedTime > sf::seconds(2.5f)) {
		// If message has expired, remove it
		mPacketHandler->removeBroadcast();

		// Continue to display next broadcast message
		if (!mPacketHandler->getBroadcastMessages().empty()) {
			mBroadcastText.setString(mPacketHandler->getBroadcastMessages().front());
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
		mPacketHandler->sendDisconnectSelf(&mSocket);
	}
}

#pragma endregion

#pragma region Packet Handling

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

void MultiplayerGameState::playerDisconnect(sf::Int32 characterIdentifier)
{
	mWorld.removeCharacter(characterIdentifier);
	mPlayers.erase(characterIdentifier);
}

void MultiplayerGameState::playerEvent(sf::Int32 characterIdentifier, sf::Int32 action)
{
	auto itr = mPlayers.find(characterIdentifier);
	if (itr != mPlayers.end())
		itr->second->handleNetworkEvent(static_cast<Player::Action>(action), mWorld.getCommandQueue());
}

void MultiplayerGameState::playerRealTimeChange(sf::Int32 characterIdentifier, sf::Int32 action, bool actionEnabled)
{
	auto itr = mPlayers.find(characterIdentifier);
	if (itr != mPlayers.end())
		itr->second->handleNetworkRealtimeChange(static_cast<Player::Action>(action), actionEnabled);
}

void MultiplayerGameState::spawnZombie(int x, int y)
{
	mWorld.addZombie(x, y);
}

void MultiplayerGameState::spawnPickup(sf::Int32 type, sf::Vector2f position)
{
	mWorld.createPickup(position, static_cast<Pickup::Type>(type));
}

void MultiplayerGameState::updateClientState(sf::Int32 characterIdentifier, sf::Vector2f characterPosition)
{
		Character* character = mWorld.getCharacter(characterIdentifier);

		if (character && (characterIdentifier != mLocalPlayerID))
		{
			sf::Vector2f interpolatedPosition = character->getPosition() + (characterPosition - character->getPosition()) * 0.1f;
			character->setPosition(interpolatedPosition);
		}
}

void MultiplayerGameState::oldUpdateClientState(sf::Packet packet)
{
	float currentWorldPosition;
	sf::Int32 aircraftCount;
	packet >> currentWorldPosition >> aircraftCount;

	float currentViewPosition = mWorld.getViewBounds().top + mWorld.getViewBounds().height;

	// Set the world's scroll compensation according to whether the view is behind or too advanced
	//mWorld.setWorldScrollCompensation(currentViewPosition / currentWorldPosition);

	for (sf::Int32 i = 0; i < aircraftCount; ++i)
	{
		sf::Vector2f aircraftPosition;
		sf::Int32 aircraftIdentifier;
		packet >> aircraftIdentifier >> aircraftPosition.x >> aircraftPosition.y;

		Character* aircraft = mWorld.getCharacter(aircraftIdentifier);
		bool isLocalPlane = std::find(mLocalPlayerIdentifiers.begin(), mLocalPlayerIdentifiers.end(), aircraftIdentifier) != mLocalPlayerIdentifiers.end();
		if (aircraft && !isLocalPlane)
		{
			sf::Vector2f interpolatedPosition = aircraft->getPosition() + (aircraftPosition - aircraft->getPosition()) * 0.1f;
			aircraft->setPosition(interpolatedPosition);
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
	bool foundLocalPlayer = mWorld.getCharacter(mLocalPlayerID);

	for (auto itr = mPlayers.begin(); itr != mPlayers.end();)
	{

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

	//TODO IF PLAYER IS DEAD SET TO SPECTATING UNTIL GAME IS OVER TO AVOID GAME ENDING ON HOST DEATH
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
	GameActions::Action gameAction;

	while (mWorld.pollGameAction(gameAction)) 
		mPacketHandler->sendGameEvent(&mSocket, gameAction);
}

void MultiplayerGameState::handlePositionUpdates()
{
	//// Regular position updates
	//if (mTickClock.getElapsedTime() > sf::seconds(1.f / 20.f)) {
	//	sf::Packet positionUpdatePacket;
	//	positionUpdatePacket << static_cast<sf::Int32>(Client::PositionUpdate);
	//	positionUpdatePacket << static_cast<sf::Int32>(mLocalPlayerIdentifiers.size());

	//	FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
	//	{
	//		if (Character* character = mWorld.getCharacter(identifier))
	//			positionUpdatePacket << identifier << character->getPosition().x << character->getPosition().y;
	//	}

	//	mSocket.send(positionUpdatePacket);
	//	mTickClock.restart();
	//}


		// OLD CODE
	if (mTickClock.getElapsedTime() > sf::seconds(1.f / 20.f))
	{
		sf::Packet positionUpdatePacket;
		positionUpdatePacket << static_cast<sf::Int32>(Client::PositionUpdate);
		positionUpdatePacket << static_cast<sf::Int32>(mLocalPlayerIdentifiers.size());

		FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
		{
			if (Character* aircraft = mWorld.getCharacter(identifier))
				positionUpdatePacket << identifier << aircraft->getPosition().x << aircraft->getPosition().y << static_cast<sf::Int32>(aircraft->getHitpoints()) << static_cast<sf::Int32>(aircraft->getGrenadeAmmo());
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