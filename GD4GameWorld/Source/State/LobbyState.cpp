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
	, mGUIContainer()
	, mClientTimeout(sf::seconds(300.f)) // 5 second timeout
	, mTimeSinceLastPacket(sf::seconds(0.f))
	, mHost(isHost)
	, mStack(stack)
	, mPacketHandler(new PacketHandler)
{
	mPacketHandler->setLobby(this);
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
			mPacketHandler->sendLoadGame(&mSocket);
		});

		mGUIContainer.pack(readyButton);
	}

	auto exitButton = std::make_shared<GUI::Button>(context);
	exitButton->setPosition(300, 500);
	exitButton->setText("Back");
	exitButton->setCallback([this]() {
		mText.setString("Disconnecting....");
		mPacketHandler->sendDisconnectSelf(&mSocket);
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
	sf::Socket::Status status = mSocket.connect(ip, ServerPort, sf::seconds(2.f));

	if (status == sf::TcpSocket::Done)
	{
		mPacketHandler->setConnected(true);
		mConnected = true;
	}
	else
	{
		mPacketHandler->setConnected(false);
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

	return true;
}

void LobbyState::updateBroadcastMessage(sf::Time elapsedTime)
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
	std::cout << "RECIEVED Load GAme " << std::endl;
	if (mHost)
	{
		//requestStackPop();
		requestStackPush(States::HostGame);
	}
	else
	{
		//requestStackPop();
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

#pragma region Getters and Setters


void LobbyState::setLocalID(sf::Int32 id)
{
	mLocalPlayerID = id;
}

sf::Int32  LobbyState::getLocalID()
{
	return mLocalPlayerID;
}
void LobbyState::increasePlayerCount()
{
	++mPlayerCount;
}

void LobbyState::decreasePlayerCount()
{
	--mPlayerCount;
}

int16_t LobbyState::getPlayerCount(int16_t playerCount)
{
	return mPlayerCount;
}

void LobbyState::setPlayerCount(int16_t playerCount)
{
	mPlayerCount = playerCount;
}

void LobbyState::RegisterGameState(sf::Int32 localId)
{
	mStack.registerState<MultiplayerGameState>(States::HostGame, true, localId);
	mStack.registerState<MultiplayerGameState>(States::JoinGame, false, localId);
}

#pragma endregion
