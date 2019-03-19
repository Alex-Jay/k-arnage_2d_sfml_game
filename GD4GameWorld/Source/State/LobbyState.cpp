#include "State/LobbyState.hpp"
#include "Component/Button.hpp"
#include "Structural/Utility.hpp"
#include "Audio/MusicPlayer.hpp"
#include "Structural/ResourceHolder.hpp"

#include <SFML/Network/IpAddress.hpp>

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
	, mGUIContainer()
	, mWindow(*context.window)
	, mClientTimeout(sf::seconds(300.f))//Loby Timeout of five min
	, mTimeSinceLastPacket(sf::seconds(0.f))
{
	sf::Texture& texture = context.textures->get(Textures::LobbyScreen);
	mBackgroundSprite.setTexture(texture);

	setDisplayText(context);

	if (isHost)
	{
		auto readyButton = std::make_shared<GUI::Button>(context);
		readyButton->setPosition(300, 600);
		readyButton->setText("Ready");
		readyButton->setCallback([this]() {

			//TODO KEEP TRACK OF READY PLAYERS
			requestStackPop();
			requestStackPush(States::Game);
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

	sf::IpAddress ip;
	if (isHost) {
		mGameServer.reset(new GameServer(sf::Vector2f(mWindow.getSize())));
		ip = "127.0.0.1";
	}
	else {
		ip = LobbyState::getAddressFromFile();
		std::string s = ip.toString();
	}

	if (mSocket.connect(ip, ServerPort, sf::seconds(5.f)) == sf::TcpSocket::Done)
		mConnected = true;
	else
		mFailedConnectionClock.restart();

	mSocket.setBlocking(false);
}

void LobbyState::setDisplayText(Context context)
{
	std::string text = "Multiplayer Lobby " + std::to_string(getConnectedPlayers()) + " Players Connected";
	mText.setFont(context.fonts->get(Fonts::Main));
	mText.setString(text);
	centerOrigin(mText);
	mText.setPosition(sf::Vector2f(context.window->getSize().x / 2u, 100));
}

void LobbyState::updateDisplayText()
{
	std::string text = "Multiplayer Lobby " + std::to_string(getConnectedPlayers()) + " Players Connected";
	mText.setString(text);
}

int8_t LobbyState::getConnectedPlayers()
{
	return mPlayers.size();
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

bool LobbyState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}

void LobbyState::returnToMenu()
{
	requestStackPop();
	requestStackPush(States::Menu);
}

#pragma region Networking Logic

void LobbyState::sendDisconnectSelf()
{
	if (!mHost && mConnected) {
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		mSocket.send(packet);
	}
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
		mSocket.send(packet);
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

void LobbyState::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
	switch (packetType) {
		// Send message to all clients
	case Server::BroadcastMessage: {
		std::string message;
		packet >> message;
		mBroadcasts.push_back(message);

		// Just added first message, display immediately
		if (mBroadcasts.size() == 1) {
			mBroadcastText.setString(mBroadcasts.front());
			centerOrigin(mBroadcastText);
			mBroadcastElapsedTime = sf::Time::Zero;
		}
	} break;

	case Server::SpawnSelf: {

		sf::Int32 characterIdentifier;
		sf::Vector2f characterPosition;
		packet >> characterIdentifier >> characterPosition.x >> characterPosition.y;

		mPlayers.push_back(lobbyPlayers(characterIdentifier, characterPosition.x, characterPosition.y));

		mLocalPlayerID = characterIdentifier;
		//std::cout << "YOU CONNECTED!!!!!!!" << std::endl;
		updateDisplayText();
	} break;

	case Server::PlayerConnect: {

		sf::Int32 characterIdentifier;
		sf::Vector2f characterPosition;
		packet >> characterIdentifier >> characterPosition.x >> characterPosition.y;

		mPlayers.push_back(lobbyPlayers(characterIdentifier, characterPosition.x, characterPosition.y));
		//std::cout << "PLAYER CONNECTED!!!!!!!" << std::endl;
		updateDisplayText();
	} break;

		//
	case Server::PlayerDisconnect: 
	{
		std::cout << "8 Client Recived Disconnect Message" << std::endl;
		sf::Int32 characterIdentifier;
		packet >> characterIdentifier;

		mPlayers.erase(
			std::remove_if(mPlayers.begin(), mPlayers.end(), [&](lobbyPlayers const & player) {
			return player.identifier == characterIdentifier;}), mPlayers.end());
		updateDisplayText();

		if (characterIdentifier == mLocalPlayerID)
		{
			returnToMenu();
		}
		//std::cout << "PLAYER DISCONNECTED!!!!!!!" << std::endl;
	} break;

	case Server::InitialState: {
		sf::Int32 characterCount;
		float worldHeight, currentScroll;

		packet >> worldHeight >> currentScroll; // TODO No NEED FOR These VAriables

		packet >> characterCount;

		for (sf::Int32 i = 0; i < characterCount; ++i) {
			sf::Int32 characterIdentifier;
			sf::Int32 hitpoints;
			sf::Int32 grenadeAmmo;
			sf::Vector2f characterPosition;
			packet >> characterIdentifier >> characterPosition.x >> characterPosition.y >> hitpoints >> grenadeAmmo;

			mPlayers.push_back(lobbyPlayers(characterIdentifier, characterPosition.x, characterPosition.y));
		}
		updateDisplayText();
	} break;
	}
}

#pragma endregion
