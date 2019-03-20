#pragma once
#include "State/State.hpp"
#include "State/StateStack.hpp"
#include "Component/Container.hpp"
#include "Entity/Character.hpp"
#include "Networking/GameServer.hpp"
#include "Networking/NetworkProtocol.hpp"
#include "Entity/Player.hpp"
#include "../SocketManager.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/Packet.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class LobbyState : public State {
public:
	sf::IpAddress getAddressFromFile();
	LobbyState(StateStack& stack, Context context, bool isHost);

	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);
	virtual void onActivate();

	void onDestroy();

private:
	typedef std::unique_ptr<Player> PlayerPtr;

	void setText(Context context);
	void setDisplayText(Context context);
	void setButtons(Context context);
	void connectToServer();
	void updateDisplayText();
	void updateBroadcastMessage(sf::Time elapsedTime);
	void handlePacket(sf::Int32 packetType, sf::Packet& packet);
	void setBroadcastMessage(sf::Packet & packet);
	void spawnSelf(sf::Packet & packet);
	void playerConnect(sf::Packet & packet);
	void playerDisconnect(sf::Packet & packet);
	void setInitialLobbyState(sf::Packet & packet);
	void startGame();
	void returnToMenu();
	void sendDisconnectSelf();
	void sendStartGame();

private:
	sf::RenderWindow& mWindow;
	sf::TcpSocket& mSocket;
	int16_t mLocalPlayerID;

	int16_t mPlayerCount;
	std::unique_ptr<GameServer> mGameServer;

	bool mConnected;
	bool mActiveState;
	bool mHasFocus;
	bool mHost;
	bool mDisconnectSelf{};

	sf::Sprite mBackgroundSprite;
	GUI::Container mGUIContainer;
	sf::Text mText;

	sf::Text mFailedConnectionText;
	sf::Clock mFailedConnectionClock;
	std::vector<std::string> mBroadcasts;
	sf::Text mBroadcastText;
	sf::Time mBroadcastElapsedTime;

	sf::Time mClientTimeout;
	sf::Time mTimeSinceLastPacket;
	StateStack& mStack;

};
