#include "State/State.hpp"
#include "State/StateStack.hpp"


State::Context::Context(sf::RenderWindow& window, sf::TcpSocket& socket, uint16_t& localID, TextureHolder& textures, FontHolder& fonts,
	MusicPlayer& music, SoundPlayer& sounds, KeyBinding& keys)
	: window(&window)
	, socket(&socket)
	, localID(0)
	, textures(&textures)
	, fonts(&fonts)
	, music(&music)
	, sounds(&sounds)
	, keys(&keys)
{
}

State::State(StateStack& stack, Context context)
	: mStack(&stack)
	, mContext(context)
{
}

State::~State()
{
}

void State::requestStackPush(States::ID stateID)
{
	mStack->pushState(stateID);
}

void State::requestStackPop()
{
	mStack->popState();
}

void State::requestStateClear()
{
	mStack->clearStates();
}

State::Context State::getContext() const
{
	return mContext;
}

void State::setLocalID(sf::Int32 id)
{
	mContext.localID = id;
}

sf::Int32 State::getLocalID()
{
	return mContext.localID;
}

void State::onActivate()
{
}

void State::onDestroy()
{
}
