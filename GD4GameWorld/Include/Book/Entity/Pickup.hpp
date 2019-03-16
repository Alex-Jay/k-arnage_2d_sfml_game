#pragma once
#include "Entity/Entity.hpp"
#include "Command/Command.hpp"
#include "Structural/ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>


class Character;

class Pickup : public Entity
{
public:
	enum class Type
	{
		HealthRefill,
		GrenadeRefill,
		FireSpread,
		FireRate,
		TypeCount
	};


public:
	Pickup(Type type, const TextureHolder& textures);

	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect() const;

	void apply(Character& player) const;


protected:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;


private:
	Type mType;
	sf::Sprite mSprite;
};
