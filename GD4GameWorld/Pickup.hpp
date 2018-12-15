#pragma once
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>


class Aircraft;

class Pickup : public Entity
{
public:
	enum class PickupID
	{
		HealthRefill,
		MissileRefill,
		FireSpread,
		FireRate,
		TypeCount
	};


public:
	Pickup(PickupID type, const TextureHolder& textures);

	virtual unsigned int	getCategory() const;
	virtual sf::FloatRect	getBoundingRect() const;

	void 					apply(Aircraft& player) const;


protected:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;


private:
	PickupID 					mType;
	sf::Sprite				mSprite;
};