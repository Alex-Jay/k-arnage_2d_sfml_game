#include "TextNode.hpp"
#include "Utility.hpp"

#include "SFML/Graphics/RenderTarget.hpp"

TextNode::TextNode(const FontHolder & fonts, const std::string & text)
{
	mText.setFont(fonts.get(FontIDs::Main));
	mText.setCharacterSize(20);
	setString(text);
}

void TextNode::setString(const std::string & text)
{
	mText.setString(text);
	centreOrigin(mText);
}

void TextNode::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const
{
	target.draw(mText, states);
}
