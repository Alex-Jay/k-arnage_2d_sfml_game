#pragma once
#include <SFML/Network/TcpSocket.hpp>

struct SocketHolder {
	SocketHolder() {}

	sf::TcpSocket socket;
};