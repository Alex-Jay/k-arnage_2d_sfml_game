#ifndef BOOK_MENUSTATE_HPP
#define BOOK_MENUSTATE_HPP

#include "State/State.hpp"
#include "Component/Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class MenuState : public State
{
	public:
								MenuState(StateStack& stack, Context context);

		virtual void			draw();
		virtual bool			update(sf::Time dt);
		virtual bool			handleEvent(const sf::Event& event);

		int getButtonFromXY(int x, int y);


	private:
		sf::Sprite				mBackgroundSprite;
		GUI::Container			mGUIContainer;
};

#endif // BOOK_MENUSTATE_HPP
