#ifndef BOOK_TEXTNODE_HPP
#define BOOK_TEXTNODE_HPP

#include "Structural/ResourceHolder.hpp"
#include "Structural/ResourceIdentifiers.hpp"
#include "Node/SceneNode.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>


class TextNode : public SceneNode
{
	public:
		explicit			TextNode(const FontHolder& fonts, const std::string& text);

		void				setString(const std::string& text);


	private:
		virtual void		drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;


	private:
		sf::Text			mText;
};

#endif // BOOK_TEXTNODE_HPP
