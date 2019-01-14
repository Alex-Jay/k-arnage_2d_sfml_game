#pragma once

#include "Player.hpp"

class PlayerManager
{
public:
	PlayerManager();
	PlayerManager(int p1ID, int p2ID);

	Player* getPlayerOne();
	Player* getPlayerTwo();

	void setPlayerOne(Player* p);
	void setPlayerTwo(Player* p);

private:
	Player* mPlayerOne;
	Player* mPlayerTwo;
};