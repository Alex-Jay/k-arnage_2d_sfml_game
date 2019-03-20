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

sf::TcpSocket CLIENT_SOCKET;

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
	, mGUIContainer()
	, mWindow(*context.window)
	, mClientTimeout(sf::seconds(300.f))//Loby Timeout of five min
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
	mFailedConnectionText.setString("FUCK YEAH");
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
			sendStartGame();
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
	if (mHost) {
		mGameServer.reset(new GameServer(sf::Vector2f(mWindow.getSize())));
		ip = "127.0.0.1";
	}
	else {
		ip = LobbyState::getAddressFromFile();
		std::string s = ip.toString();
	}

	if (CLIENT_SOCKET.connect(ip, ServerPort, sf::seconds(5.f)) == sf::TcpSocket::Done)
		mConnected = true;
	else
		mFailedConnectionClock.restart();

	CLIENT_SOCKET.setBlocking(false);
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
	// Connected to server: Handle all the network logic
	if (mConnected) {

		// TODO Remove players who disconnect
		bool foundLocalPlayer = false;

		// Handle messages from server that may have arrived
		sf::Packet packet;
		if (CLIENT_SOCKET.receive(packet) == sf::Socket::Done) {
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
	mGUIContainer.handleEvent(event);
	return false;
}

void LobbyState::onActivate()
{
	mActiveState = true;
}

void LobbyState::onDestroy()
{
	std::cout << "1 ON DESTROY" << std::endl;
	if (!mHost && mConnected) {

		std::cout << "2 Sending Destroy Packet Quit NUmber: " << static_cast<sf::Int32>(Client::Quit) << std::endl;
		// Inform server this client is dying
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		CLIENT_SOCKET.send(packet);
	}
}

void LobbyState::returnToMenu()
{
	requestStackPop();
	requestStackPush(States::Menu);
}

void LobbyState::startGame()
{
	if (mHost)
		requestStackPush(States::HostGame);
	else
		requestStackPop();
	requestStackPush(States::JoinGame);
}

void LobbyState::setDisplayText(Context context)
{
	std::string text = "Multiplayer Lobby " + std::to_string(mPlayerCount) + " Players Connected";
	mText.setFont(context.fonts->get(Fonts::Main));
	mText.setString(text);
	centerOrigin(mText);
	mText.setPosition(sf::Vector2f(context.window->getSize().x / 2u, 100));
}

#pragma endregion

#pragma region Send Packet

void LobbyState::sendStartGame()
{
	if (mHost && mConnected) {
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::StartGame);
		CLIENT_SOCKET.send(packet);
	}
}

void LobbyState::sendDisconnectSelf()
{
	if (mConnected) {
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		CLIENT_SOCKET.send(packet);
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

	case Server::SpawnSelf: {
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

	case Server::StartGame:
	{
		startGame();
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

	mPlayerCount += 1;

	updateDisplayText();

	mStack.registerState<MultiplayerGameState>(States::HostGame, true, mLocalPlayerID);
	mStack.registerState<MultiplayerGameState>(States::JoinGame, false, mLocalPlayerID);
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
		returnToMenu();
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



