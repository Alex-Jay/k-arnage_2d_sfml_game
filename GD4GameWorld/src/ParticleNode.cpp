#include "ParticleNode.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Texture.hpp"

#include <algorithm>

namespace
{
	const std::vector<ParticleData> Table = initializeParticleData();
}

ParticleNode::ParticleNode(Particle::Type type, const TextureHolder& textures)
	: SceneNode()
	, mParticles()
	, mTexture(textures.get(TextureIDs::Particle))
	, mType(type)
	, mVertexArray(sf::TriangleStrip)
	, mNeedsVertexUpdate(true)
{
}

void ParticleNode::addParticle(sf::Vector2f position)
{
	Particle particle;
	particle.position = position;
	particle.color = Table[static_cast<int>(mType)].color;
	particle.lifetime = Table[static_cast<int>(mType)].lifetime;

	mParticles.push_back(particle);
}

Particle::Type ParticleNode::getParticleType() const
{
	return mType;
}

unsigned int ParticleNode::getCategory() const
{
	return static_cast<unsigned int>(Category::ParticleSystem);
}

void ParticleNode::updateCurrent(sf::Time dt, CommandQueue & commands)
{
	//Remove expired particles
	while (!mParticles.empty() && (mParticles.front().lifetime <= sf::Time::Zero))
	{
		mParticles.pop_front();
	}
	//Decrease the lifetime for everything else
	for (Particle& particle : mParticles)
	{
		particle.lifetime -= dt;
	}

	mNeedsVertexUpdate = true;
}

void ParticleNode::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const
{
	if (mNeedsVertexUpdate)
	{
		computeVertices();
		mNeedsVertexUpdate = false;
	}

	// Apply particle texture
	states.texture = &mTexture;

	// Draw vertices
	target.draw(mVertexArray, states);
}

void ParticleNode::addVertex(float worldX, float worldY, float textCoordX, float textCoordY, const sf::Color& color) const
{
	sf::Vertex vertex;
	vertex.position = sf::Vector2f(worldX, worldY);
	vertex.texCoords = sf::Vector2f(textCoordX, textCoordY);
	vertex.color = color;

	mVertexArray.append(vertex);
}

void ParticleNode::computeVertices() const
{
	sf::Vector2f size(mTexture.getSize());
	sf::Vector2f half = size / 2.f;

	// Refill vertex array
	mVertexArray.clear();
	for(const Particle& particle : mParticles)
	{
		sf::Vector2f pos = particle.position;
		sf::Color color = particle.color;

		float ratio = particle.lifetime.asSeconds() / Table[static_cast<int>(mType)].lifetime.asSeconds();
		color.a = static_cast<sf::Uint8>(255 * std::max(ratio, 0.f));

		addVertex(pos.x - half.x, pos.y - half.y, 0.f, 0.f, color);
		addVertex(pos.x + half.x, pos.y - half.y, size.x, 0.f, color);
		addVertex(pos.x + half.x, pos.y + half.y, size.x, size.y, color);
		addVertex(pos.x - half.x, pos.y + half.y, 0.f, size.y, color);
	}
}
