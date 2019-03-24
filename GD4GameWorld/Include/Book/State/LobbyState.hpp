#pragma once
#include "State/State.hpp"
#include "State/StateStack.hpp"
#include "Component/Container.hpp"
#include "Entity/Character.hpp"
#include "Networking/GameServer.hpp"
#include "Networking/NetworkProtocol.hpp"
#include "Entity/Player.hpp"
#include "Networking/PacketHandler.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/Packet.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class PacketHandler;

class LobbyState : public State {
public:
	sf::IpAddress getAddressFromFile();
	LobbyState(StateStack& stack, Context context, bool isHost);

	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);
	virtual void onActivate();
	virtual void onDestroy();

	void updateDisplayText();
	void setLocalID(sf::Int32 id);
	sf::Int32 getLocalID();
	void increasePlayerCount();
	void decreasePlayerCount();
	int16_t getPlayerCount(int16_t playerCount);
	void setPlayerCount(int16_t playerCount);
	void RegisterGameState(sf::Int32 localId);
	void returnToMenu();
	void loadGame();
private:
	typedef std::unique_ptr<Player> PlayerPtr;

	void setText(Context context);
	void setDisplayText(Context context);
	void setButtons(Context context);
	void connectToServer();
	
	void updateBroadcastMessage(sf::Time elapsedTime);

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
	bool mGameStarted{};

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

	PacketHandler* mPacketHandler;
};
