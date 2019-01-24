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

	void setTextureMap(const TextureHolder & textures);

private:
	typedef std::array<sf::RenderTexture, 2> RenderTextureArray;

	void prepareTextures(sf::Vector2u size);
	void distort(sf::RenderTexture& output);
	void add(const sf::RenderTexture& source, const sf::RenderTexture& distort, sf::RenderTarget& output);
private:

	sf::Shader mShader;
	sf::Sprite mSprite;
	sf::Texture mDistortionMap;
	sf::Clock mTimer;

	ShaderHolder mShaders;

	sf::RenderTexture mDistortionTexture;
	RenderTextureArray mFirstPassTextures;
	RenderTextureArray mSecondPassTextures;;


	float mDistortionFactor;
	float mDistortionSpeed;
};
