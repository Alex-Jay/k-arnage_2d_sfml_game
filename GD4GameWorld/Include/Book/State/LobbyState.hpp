#pragma once
#include "State/State.hpp"
#include "Component/Container.hpp"
#include "Entity/Character.hpp"
#include "Networking/GameServer.hpp"
#include "Networking/NetworkProtocol.hpp"
#include "Entity/Player.hpp"

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
	void setDisplayText(Context context);
	void updateDisplayText();
	int8_t getConnectedPlayers();
	virtual void draw();
	virtual bool update(sf::Time dt);
	void updateBroadcastMessage(sf::Time elapsedTime);
	void handlePacket(sf::Int32 packetType, sf::Packet& packet);
	virtual bool handleEvent(const sf::Event& event);

	void returnToMenu();

	void sendDisconnectSelf();

	virtual void onActivate();
	void onDestroy();

	int getButtonFromXY(int x, int y);

private:
	void displayText(Context context);

private:
	typedef std::unique_ptr<Player> PlayerPtr;

private:

	struct lobbyPlayers {
		lobbyPlayers(int16_t identifier, int16_t startX, int16_t startY)
			: identifier(identifier)
			, startX(startX)
			, startY(startY){}

		int16_t identifier;
		int16_t startX;
		int16_t startY;
	};
	std::vector<lobbyPlayers> mPlayers;
	sf::RenderWindow& mWindow;

	sf::Sprite mBackgroundSprite;
	GUI::Container mGUIContainer;

	sf::Text mText;

	std::unique_ptr<GameServer> mGameServer;

	sf::TcpSocket mSocket;

	bool mConnected;
	bool mActiveState;
	bool mHasFocus;
	bool mHost;
	bool mDisconnectSelf{};

	int16_t mLocalPlayerID;

	sf::Text mFailedConnectionText;
	sf::Clock mFailedConnectionClock;
	std::vector<std::string> mBroadcasts;
	sf::Text mBroadcastText;
	sf::Time mBroadcastElapsedTime;

	std::vector<int8_t> mLocalPlayerIdentifiers;

	sf::Time mClientTimeout;
	sf::Time mTimeSinceLastPacket;
};
