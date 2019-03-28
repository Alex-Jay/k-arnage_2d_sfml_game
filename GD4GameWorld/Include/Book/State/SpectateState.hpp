#ifndef BOOK_SPECATESTATE_HPP
#define BOOK_SPECATESTATE_HPP

#include "State/State.hpp"
#include "Component/Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class SpectateState : public State
{
public:
	SpectateState(StateStack& stack, Context context, bool letUpdatesThrough = false);
	~SpectateState();

	virtual void		draw();
	virtual bool		update(sf::Time dt);
	virtual bool		handleEvent(const sf::Event& event);


private:
	//sf::Sprite			mBackgroundSprite;
	sf::Text			mCurrentlySpectating;
	GUI::Container 		mGUIContainer;
	bool				mLetUpdatesThrough;
};

#endif // BOOK_SPECATESTATE_HPP