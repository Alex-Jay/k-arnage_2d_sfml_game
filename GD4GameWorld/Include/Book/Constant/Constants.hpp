#pragma once
#include<string>
//++++++++++++ MAP DATA ++++++++++++++
const std::string DESSERTMAP = "Media/map.txt";
const int DESSERT_TILE_WIDTH = 128;
const int DESSERT_TILE_HEIGHT = 128;
//++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ ANIMATION RECTANGLES ++++++++++++++
const int PLAYER_MOVE_ANIMATION_HEIGHT = 156;
const int PLAYER_MOVE_ANIMATION_WIDTH = 263;

const int PLAYER_DEATH_ANIMATION_HEIGHT = 434;
const int PLAYER_DEATH_ANIMATION_WIDTH = 550;

const int ZOMBIE_MOVE_ANIMATION_HEIGHT = 40;
const int ZOMBIE_MOVE_ANIMATION_WIDTH = 77;

const int ZOMBIE_DEATH_ANIMATION_HEIGHT = 88;
const int ZOMBIE_DEATH_ANIMATION_WIDTH = 150;
//++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ PLAYER CONSTANTS ++++++++++++++++++
const int ZOMBIE_KILL_MULTIPLIER = 10;
const int PLAYERHITPOINTS = 100;
const int GRENADEPICKUPAMMOVALUE = 3;
const int PLAYERSPEED = 260;
const float PLAYERFIREINTERVAL = 1.f;
const float HEALTHPICKUPREPAIRVALUE = 17.5f;
const int FIRERATELEVEL = 2;
const float MAXFIRERATELEVEL = 3.f;
const int MAXSPREADLEVEL = 1;
const int SPREADLEVEL = 1;
const int GRENADEAMMO = 2;
//+++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ ENEMY CONSTANTS ++++++++++++++++++++
const float ZOMBIEHITPOINTS = 100.f;
const float ZOMBIESPEED = 160.f;
const float ZOMBIEATTACKDAMAGE = 19.5f;
const float ZOMBIEATTACKDELAY = 600.f; // In miliseconds

const float LAVA_DAMAGE = 2.f;
//+++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ Projectile CONSTANTS ++++++++++++++++++++
const float BULLETDAMAGE = 17.5f;
const float BULLETSPEED = 1200.f;
const float GRENADEDAMAGE = 130.f;
const float GRENADESPEED = 300.f;
const float GRENADBLASTRADIOUS = 25.f;
const float LEVEL_ZOOM_FACTOR = 1.5f;
//+++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ GENERAL CONSTANTS ++++++++++++++++++
const float ROTATION_SPEED = 15.f;
const float M_PI = 3.14159265f;
const float APPROACHRATE = 200;
//++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ COLOUR CONSTANTS ++++++++++++++++++
const int BLACK = 0x000000ff;
const int RED = 0Xb20000ff;
const int GREEN = 0X008432ff;
const int BLUE = 0X0056adff;
const int ORANGE = 0Xffa100ff;


//+++++++++++ CONTROLLER CONSTANS ++++++++++++++++++
const float TRIGGER_THRESHOLD = 70.0f;
const float THUMBSTICK_THRESHOLD = 50.0f;

//++++++++++++++++++++++++++++++++++++++++++++++++++

