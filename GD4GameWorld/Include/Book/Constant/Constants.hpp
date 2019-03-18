#pragma once
#include<string>
//++++++++++++ MAP DATA ++++++++++++++
const std::string DESSERTMAP = "Media/map.txt";
const int DESSERT_TILE_WIDTH = 128;
const int DESSERT_TILE_HEIGHT = 128;
//++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ ANIMATION RECTANGLES ++++++++++++++
const int16_t PLAYER_MOVE_ANIMATION_HEIGHT = 156;
const int16_t PLAYER_MOVE_ANIMATION_WIDTH = 263;

const int16_t PLAYER_DEATH_ANIMATION_HEIGHT = 434;
const int16_t PLAYER_DEATH_ANIMATION_WIDTH = 550;

const int16_t ZOMBIE_MOVE_ANIMATION_HEIGHT = 40;
const int16_t ZOMBIE_MOVE_ANIMATION_WIDTH = 77;

const int16_t ZOMBIE_DEATH_ANIMATION_HEIGHT = 88;
const int16_t ZOMBIE_DEATH_ANIMATION_WIDTH = 150;
//++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ PLAYER CONSTANTS ++++++++++++++++++
const int16_t ZOMBIE_KILL_MULTIPLIER = 10;
const int16_t PLAYERHITPOINTS = 100;
const int16_t GRENADEPICKUPAMMOVALUE = 3;
const int16_t PLAYERSPEED = 260;
const int8_t PLAYERFIREINTERVAL = 1;
const int8_t HEALTHPICKUPREPAIRVALUE = 20;
const int8_t FIRERATELEVEL = 2;
const int8_t MAXFIRERATELEVEL = 3;
const int8_t MAXSPREADLEVEL = 1;
const int8_t SPREADLEVEL = 1;
const int8_t GRENADEAMMO = 2;
//+++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ ENEMY CONSTANTS ++++++++++++++++++++
const int16_t ZOMBIEHITPOINTS = 40;
const int16_t ZOMBIESPEED = 100;
const int16_t ZOMBIEATTACKDAMAGE = 20;
const int16_t ZOMBIEATTACKDELAY = 600; // In miliseconds

//+++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ Projectile CONSTANTS ++++++++++++++++++++
const int8_t BULLETDAMAGE = 10;
const int16_t BULLETSPEED = 1200;
const int8_t GRENADEDAMAGE = 130;
const int16_t GRENADESPEED = 300;
const int8_t GRENADBLASTRADIOUS = 25;
//+++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ GENERAL CONSTANTS ++++++++++++++++++
const int8_t ROTATION_SPEED = 15;
const float M_PI = 3.14159265f;
const int8_t APPROACHRATE = 200;
//++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ World CONSTANTS ++++++++++++++++++
const float LEVEL_ZOOM_FACTOR = 1.5f;
const int16_t WORLD_WIDTH = 3328;
const int16_t WORLD_HEIGHT = 2688;
const int8_t LAVA_DAMAGE = 2;
const int8_t MAX_ALIVE_ZOMBIES = 8;

const int16_t MIN_ZOMBIE_SPAWN_TIME = 5000;
const int16_t MAX_ZOMBIE_SPAWN_TIME = 15000;
//++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++ COLOUR CONSTANTS ++++++++++++++++++
const int BLACK = 0x000000ff;
const int RED = 0Xb20000ff;
const int GREEN = 0X008432ff;
const int BLUE = 0X0056adff;
const int ORANGE = 0Xffa100ff;


//+++++++++++ CONTROLLER CONSTANS ++++++++++++++++++
const int8_t TRIGGER_THRESHOLD = 70;
const int8_t THUMBSTICK_THRESHOLD = 50;

//++++++++++++++++++++++++++++++++++++++++++++++++++

