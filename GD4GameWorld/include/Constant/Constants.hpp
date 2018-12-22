#pragma once

//++++++++++++ ANIMATION RECTANGLES ++++++++++++++
const int PLAYERANIMATIONRECTWIDTH = 259;
const int PLAYERANIMATIONRECTHEIGHT = 153;

const int ZOMBIEANIMATIONRECTWIDTH = 259;
const int ZOMBIEANIMATIONRECTHEIGHT = 153;

//++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++ PLAYER CONSTANTS ++++++++++++++++++
const int PLAYERHITPOINTS = 100;
const int GRENADEPICKUPAMMOVALUE = 3;
const float PLAYERSPEED = 300.f;
const float PLAYERFIREINTERVAL = 1.f;
const int HEALTHPICKUPREPAIRVALUE = 25;
const int FIRERATELEVEL = 1;
const float MAXFIRERATELEVEL = 10.f;
const int MAXSPREADLEVEL = 3;
const int SPREADLEVEL = 1;
const int GRENADEAMMO = 2;
//+++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++ ENEMY CONSTANTS ++++++++++++++++++++
const int ZOMBIEHITPOINTS = 20;
const int ZOMBIESPEED = 40;
//+++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++ Projectile CONSTANTS ++++++++++++++++++++
const int BULLETDAMAGE = 10;
const int BULLETSPEED = 800;

const int GRENADEDAMAGE = 200;
const int GRENADESPEED = 300;
const int GRENADBLASTRADIOUS = 25;
const float LEVEL_ZOOM_FACTOR = 1.5f;

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++ GENERAL CONSTANTS ++++++++++++++++++
const float ROTATION_SPEED = 200.f;
const float M_PI = 3.14159265f;
const float APPROACHRATE = 200;
//++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++ COLOUR CONSTANTS ++++++++++++++++++

const sf::Uint32 BLACK = 0x000000ff;
const sf::Uint32 RED = 0Xb20000ff;
const sf::Uint32 GREEN = 0X008432ff;
const sf::Uint32 BLUE = 0X0056adff;
const sf::Uint32 ORANGE = 0Xffa100ff;



//+++++++++++ CONTROLLER CONSTANS ++++++++++++++++++
const float TRIGGER_THRESHOLD = 70.0f;
const float THUMBSTICK_THRESHOLD = 50.0f;