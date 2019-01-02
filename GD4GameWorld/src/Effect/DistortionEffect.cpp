#include "DistortionEffect.hpp"
#include <iostream>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Clock.hpp>
#include "Utility.hpp"

DistortionEffect::DistortionEffect()
	: mDistortionFactor(0.05f)
	, mRiseFactor(0.1f)
{
	//mShaders.load(Shaders::Distortion, "Media/Shaders/Distortion.frag", sf::Shader::Fragment);
	if (!mShader.loadFromFile("Media/Shaders/Distortion.frag", sf::Shader::Fragment))
	{
		throw std::runtime_error("Failed to load Media/Shaders/Distortion.frag");
	}
}

void DistortionEffect::setTextureMap(const TextureHolder & textures)
{
	mDistortionMap = textures.get(TextureIDs::DistortionMap);
	mDistortionMap.setRepeated(true);
	mDistortionMap.setSmooth(true);
	mShader.setUniform("currentTexture", sf::Shader::CurrentTexture);
	mShader.setUniform("distortionMapTexture", mDistortionMap);
}

void DistortionEffect::apply(const sf::RenderTexture& input, sf::RenderTarget& output)
{
	mSprite.setTexture(input.getTexture());

	centreOrigin(mSprite);
	mShader.setUniform("time", mTimer.getElapsedTime().asSeconds());
	mShader.setUniform("distortionFactor", mDistortionFactor);
	mShader.setUniform("riseFactor", mRiseFactor);

	output.clear();
	mSprite.setPosition(output.getSize().x / 2., output.getSize().y / 2.);
	output.draw(mSprite, &mShader);
}




