#pragma once
#include "PostEffect.hpp"
#include "ResourceIdentifiers.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <array>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>


class DistortionEffect : public PostEffect
{
public:
	DistortionEffect();

	virtual void apply(const sf::RenderTexture& input, sf::RenderTarget& output);


private:
	typedef std::array<sf::RenderTexture, 2> RenderTextureArray;

private:
	ShaderHolder mShaders;

	sf::RenderTexture mBrightnessTexture;
	RenderTextureArray mFirstPassTextures;
	RenderTextureArray mSecondPassTextures;

	sf::Shader shader;
	sf::Texture texture;
	sf::Sprite sprite;
	sf::Texture distortionMap;
	sf::Clock timer;

	float distortionFactor;
	float riseFactor;
};
