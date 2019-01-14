#include "PlayerManager.hpp"

PlayerManager::PlayerManager()
	: mPlayerOne()
	, mPlayerTwo()
{}

PlayerManager::PlayerManager(int p1ID, int p2ID)
{
	mPlayerOne = new Player(p1ID);
	mPlayerTwo = new Player(p2ID);
}

Player* PlayerManager::getPlayerOne()
{
	return mPlayerOne;
}

Player* PlayerManager::getPlayerTwo()
{
	return mPlayerTwo;
}

void PlayerManager::setPlayerOne(Player * p)
{
	mPlayerOne = p;
}

void PlayerManager::setPlayerTwo(Player * p)
{
	mPlayerTwo = p;
}
