#include "Effect/DistortionEffect.hpp"
#include <iostream>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Clock.hpp>
#include <Book/Utility.hpp>


// Frag File and tutorial from Here https://www.lucidarme.me/sfml-distorsion-shader/

DistortionEffect::DistortionEffect()
	: mDistortionFactor(0.05f)
	, mDistortionSpeed(0.1f)
{
	//Had trouble loading shader from mShaders, due to second parameter of (shaderType) required
	//mShaders.load(Shaders::Distortion, "Media/Shaders/Distortion.frag", sf::Shader::Fragment);
	if (!mShader.loadFromFile("Media/Shaders/Distortion.frag", sf::Shader::Fragment))
	{
		throw std::runtime_error("Failed to load Media/Shaders/Distortion.frag");
	}

	//mShaders.load(Shaders::AddPass, "Media/Shaders/Fullpass.vert", "Media/Shaders/Add.frag");
}

void DistortionEffect::setTextureMap(const TextureHolder & textures)
{
	mDistortionMap = textures.get(Textures::ID::DistortionMap);
	mDistortionMap.setRepeated(true);
	mDistortionMap.setSmooth(true);
	mShader.setUniform("currentTexture", sf::Shader::CurrentTexture);
	mShader.setUniform("distortionMapTexture", mDistortionMap);
}

void DistortionEffect::apply(const sf::RenderTexture& input, sf::RenderTarget& output)
{

	//Set Sprites texture to the input, this will be the water background
	mSprite.setTexture(input.getTexture());
	//centreOrigin(mSprite);

	//Set Properties of the Distortion shader
	//Time controls the movement of the distortion effect
	mShader.setUniform("time", mTimer.getElapsedTime().asSeconds());
	//DistortionFactor is the amount of distortion applied
	mShader.setUniform("distortionFactor", mDistortionFactor);
	//DistortionSpeed is the speed of the distortion
	mShader.setUniform("distortionSpeed", mDistortionSpeed);

	output.clear();
	//Center the Sprite Position
	mSprite.setPosition(output.getSize().x / 2., output.getSize().y / 2.);
	output.draw(mSprite, &mShader);

}

//+++++++++++++++++++++++++++++++++++++++++++
//Below is commented out version of the apply() method that I could not get to work as I struggled to understand the implimentation of the code.
//This is where I was trying to call applyShader in PostEffect Using Vertex arrays.
//if below code is uncommented and above 3 lines commented out, it draws the watertexture but with no movement

//void DistortionEffect::apply(const sf::RenderTexture& input, sf::RenderTarget& output)
//{
//
//	//Set Sprites texture to the input, this will be the water background
//	mSprite.setTexture(input.getTexture());
//	centreOrigin(mSprite);
//
//	prepareTextures(input.getSize());
//
//	distort(mFirstPassTextures[0]);
//	distort(mSecondPassTextures[0]);
//
//	add(mFirstPassTextures[0], mSecondPassTextures[0], mFirstPassTextures[1]);
//	mFirstPassTextures[1].display();
//	add(input, mFirstPassTextures[1], output);
//}

void DistortionEffect::prepareTextures(sf::Vector2u size)
{
	if (mDistortionTexture.getSize() != size)
	{
		mDistortionTexture.create(size.x, size.y);
		mDistortionTexture.setSmooth(true);

		mFirstPassTextures[0].create(size.x / 2, size.y / 2);
		mFirstPassTextures[0].setSmooth(true);
		mFirstPassTextures[1].create(size.x / 2, size.y / 2);
		mFirstPassTextures[1].setSmooth(true);

		mSecondPassTextures[0].create(size.x / 4, size.y / 4);
		mSecondPassTextures[0].setSmooth(true);
		mSecondPassTextures[1].create(size.x / 4, size.y / 4);
		mSecondPassTextures[1].setSmooth(true);
	}
}

void DistortionEffect::distort(sf::RenderTexture& output)
{

	mShader.setUniform("time", mTimer.getElapsedTime().asSeconds());
	mShader.setUniform("distortionFactor", mDistortionFactor);
	mShader.setUniform("distortionSpeed", mDistortionSpeed);
	applyShader(mShader, output);
	output.display();
}


void DistortionEffect::add(const sf::RenderTexture& source, const sf::RenderTexture& distort, sf::RenderTarget& output)
{
	sf::Shader& adder = mShaders.get(Shaders::ID::AddPass);

	adder.setUniform("source", source.getTexture());
	adder.setUniform("distort", distort.getTexture());
	applyShader(adder, output);
}
