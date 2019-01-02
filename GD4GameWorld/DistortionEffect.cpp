#include "DistortionEffect.hpp"
#include <iostream>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Clock.hpp>

DistortionEffect::DistortionEffect()
{
	texture.loadFromFile("Media/Textures/Water.jpg");
	texture.setSmooth(true);




	shader.loadFromFile("Media/Shaders/Distortion.frag", sf::Shader::Fragment);

	distortionMap.loadFromFile("Media/Textures/distortion_map.png");
	// It is important to set repeated to true to enable scrolling upwards
	distortionMap.setRepeated(true);
	// Setting smooth to true lets us use small maps even on larger images
	distortionMap.setSmooth(true);

	// Set texture to the shader
	shader.setUniform("currentTexture", sf::Shader::CurrentTexture);
	shader.setUniform("distortionMapTexture", distortionMap);

	timer.restart();
}

void DistortionEffect::apply(const sf::RenderTexture& input, sf::RenderTarget& output)
{
	sprite.setTexture(input.getTexture());
	sf::FloatRect spriteSize = sprite.getGlobalBounds();
	// Set origin in the middle of the sprite
	sprite.setOrigin(spriteSize.width / 2., spriteSize.height / 2.);
	shader.setUniform("time", timer.getElapsedTime().asSeconds());
	shader.setUniform("distortionFactor", 0.05f);
	shader.setUniform("riseFactor", 0.1f);

	// Clear the window and apply grey background
	output.clear(sf::Color(127, 127, 127));

	// Draw the sprite and apply shader
	sprite.setPosition(output.getSize().x / 2., output.getSize().y / 2.);

	output.draw(sprite, &shader);
}

