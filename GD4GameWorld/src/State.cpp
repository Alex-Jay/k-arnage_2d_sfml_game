#include "State.hpp"
#include "StateStack.hpp"

State::State(StateStack & stack, Context context) 
	: mStack(&stack), mContext(context)
{
}

State::~State()
{
}

void State::requestStackPush(StateIDs stateID)
{
	mStack->pushState(stateID);
}

void State::requestStackPop()
{
	mStack->popState();
}

void State::requestStackClear()
{
	mStack->clearStates();
}

State::Context State::getContext() const
{
	return mContext;
}

State::Context::Context(sf::RenderWindow & window, TextureHolder & textures, FontHolder & fonts, Player & player, MusicPlayer& music, SoundPlayer& sounds) 
	: window(&window)
	, textures(&textures)
	, fonts(&fonts)
	, player(&player)
	, music(&music)
	, sounds(&sounds)
{
}
