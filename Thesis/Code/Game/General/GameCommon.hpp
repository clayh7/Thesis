#pragma once

#define STATIC
#define BIT(x)(1<<(x))
#include "Engine/Net/Session/NetSession.hpp"
typedef unsigned char byte_t;


//-------------------------------------------------------------------------------------------------
#define GAME_VERSION 12
/* Version Log
12: Community Playtest
11: Thesis Playtest
10: Updated UI
9:	Map/Player Saving
4:	Adding Rocks
3:	Adding Bullets
2:	Adding Players
1:	First version
*/


//-------------------------------------------------------------------------------------------------
class Color;
enum ePlayerClass : byte_t;


//-------------------------------------------------------------------------------------------------
// Global Constants
//-------------------------------------------------------------------------------------------------
extern char const * CLAY_FONT;
extern char const * SPRITE_ID_ERROR;
extern char const * STRING_POSITION;
extern char const * STRING_VELOCITY;
extern char const * STRING_ROTATION;
extern char const * STRING_HEALTH;
extern char const * STRING_SHIELD;
extern char const * STRING_ENERGY;
extern char const * STRING_SPAWNER_ID;
extern char const * STRING_ITEM_CODE;
extern char const * STRING_AMOUNT;
extern char const * STRING_EMOTE_TYPE;
extern char const * STRING_ALLY_TYPE;
extern char const * STRING_ENEMY_TYPE;
extern char const * STRING_ENEMY_LEVEL;
extern char const * SOUND_FIRE1_FILE;
extern char const * SOUND_FIRE2_FILE;
extern char const * SOUND_FIRE3_FILE;
extern char const * SOUND_FIRE4_FILE;
extern char const * SOUND_FIRE5_FILE;
extern size_t const SOUND_FIRE_COUNT;
extern char const * SOUND_EXPLODE1_FILE;
extern char const * SOUND_EXPLODE2_FILE;
extern char const * SOUND_EXPLODE3_FILE;
extern size_t const SOUND_EXPLODE_COUNT;
extern char const * SOUND_HIT1_FILE;
extern char const * SOUND_HIT2_FILE;
extern char const * SOUND_HIT3_FILE;
extern size_t const SOUND_HIT_COUNT;
extern int const LAYER_BACKGROUND;
extern int const LAYER_FX;
extern int const LAYER_CRYSTAL;
extern int const LAYER_PLAYER;
extern int const LAYER_WEAPON;
extern int const LAYER_DEBUG_PLAYER;
extern int const LAYER_EMOTE;
extern int const LAYER_UI_GAME;
extern int const LAYER_UI_FOREGROUND;
extern float const RADIUS_TO_SCALE;


//-------------------------------------------------------------------------------------------------
// Global Enums
//-------------------------------------------------------------------------------------------------
//Needs to be updated everytime a new GameObject type is created
enum eNetGameObjectType : byte_t
{
	eNetGameObjectType_BULLET,
	eNetGameObjectType_ROCK,
	eNetGameObjectType_EMOTE,
	eNetGameObjectType_PICKUP,
	eNetGameObjectType_PLAYERSHIP,
	eNetGameObjectType_ALLYSHIP,
	eNetGameObjectType_ENEMYSHIP,
};


//-------------------------------------------------------------------------------------------------
enum eGameButton : uint16_t
{
	eGameButton_UP,
	eGameButton_DOWN,
	eGameButton_LEFT,
	eGameButton_RIGHT,
};


//-------------------------------------------------------------------------------------------------
enum eGameEvent : byte_t
{
	eGameEvent_PRESS_SHOOT,
	eGameEvent_UPGRADE_HULL,
	eGameEvent_UPGRADE_HEALTH,
	eGameEvent_UPGRADE_SHIELD,
	eGameEvent_UPGRADE_ENERGY,
	eGameEvent_EJECT_ITEM,
	eGameEvent_EQUIP_PRIMARY,
	eGameEvent_REMOVE_PRIMARY,
	eGameEvent_FEED_CRYSTAL,
	eGameEvent_WARP_DRIVE,

	eGameEvent_CHOOSE_CLASS_FIGHTER,
	eGameEvent_CHOOSE_CLASS_WIZARD,
	eGameEvent_CHOOSE_CLASS_ROGUE,
	eGameEvent_CHOOSE_CLASS_CULTIST,

	eGameEvent_EMOTE_HAPPY,
	eGameEvent_EMOTE_SAD,
	eGameEvent_EMOTE_ANGRY,
	eGameEvent_EMOTE_WHOOPS,
	eGameEvent_EMOTE_HELP,
};


//-------------------------------------------------------------------------------------------------
enum eGameNetSessionError
{
	eGameNetSessionError_WRONG_PASSWORD = eNetSessionError_COUNT,
	eGameNetSessionError_LOGGED_IN_AS_HOST,
};


//-------------------------------------------------------------------------------------------------
enum eNetGameMessageType : byte_t
{
	eNetGameMessageType_NET_PLAYER_CREATE = eNetMessageType_COUNT,
	eNetGameMessageType_NET_PLAYER_DESTROY,
	eNetGameMessageType_NET_PLAYER_UPDATE,
	eNetGameMessageType_NET_OBJECT_CREATE,
	eNetGameMessageType_NET_OBJECT_UPDATE,
	eNetGameMessageType_NET_OBJECT_DESTROY,
	eNetGameMessageType_GAME_STATE_UPDATE,
	eNetGameMessageType_RELIABLE_INPUT,
	eNetGameMessageType_UNRELIABLE_INPUT,
};


//-------------------------------------------------------------------------------------------------
// Global Functions
//-------------------------------------------------------------------------------------------------
//#TODO: should this be a static in Game()?
Color GetColorFromIndex( byte_t netIndex );
Color GetColorFromClass( ePlayerClass const & classIndex );