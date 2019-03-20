#include "State/LobbyState.hpp"
#include "Component/Button.hpp"
#include "Structural/Utility.hpp"
#include "Audio/MusicPlayer.hpp"
#include "Structural/ResourceHolder.hpp"
#include "State/MultiplayerGameState.hpp"
#include <SFML/Network/IpAddress.hpp>
#include "Constant/Constants.hpp"
#include <string>
#include <fstream>
#include <iostream>

sf::IpAddress LobbyState::getAddressFromFile()
{
	{ // Try to open existing file (RAII block)
		std::ifstream inputFile("ip.txt");
		std::string ipAddress;
		if (inputFile >> ipAddress)
			return ipAddress;
	}

	// If open/read failed, create new file
	std::ofstream outputFile("ip.txt");
	std::string localAddress = "127.0.0.1";
	outputFile << localAddress;

	return localAddress;
}

LobbyState::LobbyState(StateStack& stack, Context context, bool isHost)
	: State(stack, context)
	, mWindow(*context.window)
	, mSocket(*context.socket)
	, mLocalPlayerID(*context.localID)
	, mGUIContainer()
	, mClientTimeout(sf::seconds(300.f)) // 5 second timeout
	, mTimeSinceLastPacket(sf::seconds(0.f))
	, mHost(isHost)
	, mStack(stack)
{
	setText(context);
	setButtons(context);
	connectToServer();

}

#pragma region Initialization

void LobbyState::setText(Context context)
{
	sf::Texture& texture = context.textures->get(Textures::LobbyScreen);
	mBackgroundSprite.setTexture(texture);

	mBroadcastText.setFont(context.fonts->get(Fonts::Main));
	mBroadcastText.setPosition(1024.f / 2, 400.f);

	// We reuse this text for "Attempt to connect" and "Failed to connect" messages
	mFailedConnectionText.setFont(context.fonts->get(Fonts::Main));
	mFailedConnectionText.setCharacterSize(35);
	mFailedConnectionText.setFillColor(sf::Color::White);
	centerOrigin(mFailedConnectionText);
	mFailedConnectionText.setPosition(mWindow.getSize().x / 2.f, mWindow.getSize().y / 2.f);

	// Render a "establishing connection" frame for user feedback
	mWindow.clear(sf::Color::Black);
	mWindow.draw(mFailedConnectionText);
	mWindow.display();
	mFailedConnectionText.setString("Could not connect to the remote server!");
	centerOrigin(mFailedConnectionText);

	setDisplayText(context);
}

void LobbyState::setButtons(Context context)
{
	if (mHost)
	{
		auto readyButton = std::make_shared<GUI::Button>(context);
		readyButton->setPosition(300, 600);
		readyButton->setText("Ready");
		readyButton->setCallback([this]()
		{
			sendLoadGame();
		});

		mGUIContainer.pack(readyButton);
	}

	auto exitButton = std::make_shared<GUI::Button>(context);
	exitButton->setPosition(300, 500);
	exitButton->setText("Back");
	exitButton->setCallback([this]() {
		mText.setString("Disconnecting....");
		sendDisconnectSelf();
	});

	mGUIContainer.pack(exitButton);
}

void LobbyState::connectToServer()
{
	sf::IpAddress ip;

	if (mHost)
	{
		mGameServer.reset(new GameServer(sf::Vector2f(mWindow.getSize())));
		ip = sf::IpAddress::getLocalAddress();
	}
	else
	{
		ip = LobbyState::getAddressFromFile();
		std::string s = ip.toString();
	}

	mSocket.setBlocking(true);
	sf::Socket::Status status = mSocket.connect(ip, ServerPort, sf::seconds(5.f));

	if (status == sf::TcpSocket::Done)
	{
		mConnected = true;
	}
	else
	{
		mConnected = false;
		mFailedConnectionClock.restart();
	}

	mSocket.setBlocking(false);
}
#pragma endregion

#pragma region Update

void LobbyState::updateDisplayText()
{
	std::string text = "Multiplayer Lobby " + std::to_string(mPlayerCount) + " Players Connected";
	mText.setString(text);
}

void LobbyState::draw()
{
	if (mConnected) {

		// Broadcast messages in default view
		mWindow.setView(mWindow.getDefaultView());
		mWindow.draw(mBackgroundSprite);
		mWindow.draw(mGUIContainer);
		mWindow.draw(mText);

		if (!mBroadcasts.empty())
			mWindow.draw(mBroadcastText);
	}
	else {
		mWindow.draw(mFailedConnectionText);
	}
}

bool LobbyState::update(sf::Time dt)
{
	if (!mGameStarted)
	{
		// Connected to server: Handle all the network logic
		if (mConnected) {

			// TODO Remove players who disconnect
			bool foundLocalPlayer = false;

			// Handle messages from server that may have arrived
			sf::Packet packet;
			if (mSocket.receive(packet) == sf::Socket::Done) {
				mTimeSinceLastPacket = sf::seconds(0.f);
				sf::Int32 packetType;
				packet >> packetType;
				handlePacket(packetType, packet);
			}
			else {
				// Check for timeout with the server
				if (mTimeSinceLastPacket > mClientTimeout) {
					mConnected = false;

					mFailedConnectionText.setString("Lost connection to server");
					centerOrigin(mFailedConnectionText);

					mFailedConnectionClock.restart();
				}
			}

			updateBroadcastMessage(dt);
			mTimeSinceLastPacket += dt;
		}

		// Failed to connect and waited for more than 5 seconds: Back to menu
		else if (mFailedConnectionClock.getElapsedTime() >= sf::seconds(5.f)) {
			requestStateClear();
			requestStackPush(States::Menu);
		}
	}

	return true;
}

void LobbyState::updateBroadcastMessage(sf::Time elapsedTime)
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

bool LobbyState::handleEvent(const sf::Event& event)
{
	if (!mGameStarted)
		mGUIContainer.handleEvent(event);

	return false;
}

void LobbyState::onActivate()
{
	mActiveState = true;
}

void LobbyState::onDestroy()
{
	if (!mHost && mConnected)
	{
		// Inform server that host has left or disconnected
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		mSocket.send(packet);
	}
}

void LobbyState::returnToMenu()
{
	requestStackPop();
	requestStackPush(States::Menu);
}

void LobbyState::loadGame()
{
	//std::cout << "RECIEVED Load GAme " << std::endl;
	if (mHost)
	{
		//requestStackPop();
		requestStackPush(States::HostGame);
	}
	else
	{
		requestStackPop();
		requestStackPush(States::JoinGame);
	}

	mGameStarted = true;
}

void LobbyState::setDisplayText(Context context)
{
	std::string text = "Multiplayer Lobby " + std::to_string(mPlayerCount) + " Player(s) Connected";
	mText.setFont(context.fonts->get(Fonts::Main));
	mText.setString(text);
	centerOrigin(mText);
	mText.setPosition(sf::Vector2f(context.window->getSize().x / 2u, 100));
}

#pragma endregion

#pragma region Send Packet

void LobbyState::sendLoadGame()
{
	//std::cout << "1 SEND LOAD GAME"  << std::endl;
	if (mHost && mConnected) {
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::LoadGame);
		mSocket.send(packet);
	}
}

void LobbyState::sendDisconnectSelf()
{
	if (mConnected) {
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		mSocket.send(packet);
	}
}

#pragma endregion

#pragma region Handle Packet

void LobbyState::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
	switch (packetType) {
		// Send message to all clients
	case Server::BroadcastMessage: {
		setBroadcastMessage(packet);
	} break;

	case Server::JoinLobby: {
		spawnSelf(packet);
	} break;

	case Server::PlayerConnect: {
		playerConnect(packet);
	} break;

	case Server::PlayerDisconnect:
	{
		playerDisconnect(packet);
	} break;

	case Server::InitialState: {
		setInitialLobbyState(packet);
	} break;

	case Server::LoadGame:
	{
		loadGame();
	} break;
	}
}

void LobbyState::setBroadcastMessage(sf::Packet& packet)
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

void LobbyState::spawnSelf(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;

	packet >> characterIdentifier;

	mLocalPlayerID = characterIdentifier;

	++mPlayerCount;

	updateDisplayText();
}

void LobbyState::playerConnect(sf::Packet& packet)
{
	++mPlayerCount;
	updateDisplayText();
}

void LobbyState::playerDisconnect(sf::Packet& packet)
{
	sf::Int32 characterIdentifier;
	packet >> characterIdentifier;
	
	if (characterIdentifier == mLocalPlayerID)
	{
		mSocket.disconnect();
		returnToMenu();
	}
	else
	{
		--mPlayerCount;
		updateDisplayText();
	}

}

void LobbyState::setInitialLobbyState(sf::Packet& packet)
{
	sf::Int32 characterCount;

	packet >> characterCount;
	
	mPlayerCount = characterCount;

	updateDisplayText();
}

#pragma endregion