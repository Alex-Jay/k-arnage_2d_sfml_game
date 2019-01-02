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
	DistortionEffect(const TextureHolder& textures);

	virtual void apply(const sf::RenderTexture& input, sf::RenderTarget& output);

	void setTextureMap(const TextureHolder & textures);

private:
	typedef std::array<sf::RenderTexture, 2> RenderTextureArray;

private:

	sf::Shader mShader;
	sf::Sprite mSprite;
	sf::Texture mDistortionMap;
	sf::Clock mTimer;

	float mDistortionFactor;
	float mRiseFactor;
};
