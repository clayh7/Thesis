#include "Game/General/GameCommon.hpp"

#include <math.h>
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UIButton.hpp"
#include "Engine/UISystem/UIBox.hpp"
#include "Engine/UISystem/UILabel.hpp"
#include "Game/GameObjects/Player/Player.hpp"


//-------------------------------------------------------------------------------------------------
char const * CLAY_FONT = "ClayFont";
extern char const * SPRITE_ID_ERROR = "error";
char const * STRING_POSITION = "position";
char const * STRING_VELOCITY = "velocity";
char const * STRING_ROTATION = "rotation";
char const * STRING_HEALTH = "health";
char const * STRING_SHIELD = "shield";
char const * STRING_ENERGY = "energy";
char const * STRING_SPAWNER_ID = "spawnerID";
char const * STRING_ITEM_CODE = "itemCode";
char const * STRING_AMOUNT = "amount";
char const * STRING_EMOTE_TYPE = "emoteType";
char const * STRING_ALLY_TYPE = "allyType";
char const * STRING_ENEMY_TYPE = "enemyType";
char const * STRING_ENEMY_LEVEL = "enemyLevel";
char const * SOUND_FIRE1_FILE = "Data/Audio/fire1.wav";
char const * SOUND_FIRE2_FILE = "Data/Audio/fire2.wav";
char const * SOUND_FIRE3_FILE = "Data/Audio/fire3.wav";
char const * SOUND_FIRE4_FILE = "Data/Audio/fire4.wav";
char const * SOUND_FIRE5_FILE = "Data/Audio/fire5.wav";
size_t const SOUND_FIRE_COUNT = 5;
char const * SOUND_EXPLODE1_FILE = "Data/Audio/explode1.wav";
char const * SOUND_EXPLODE2_FILE = "Data/Audio/explode2.wav";
char const * SOUND_EXPLODE3_FILE = "Data/Audio/explode3.wav";
size_t const SOUND_EXPLODE_COUNT = 3;
char const * SOUND_HIT1_FILE = "Data/Audio/hit1.wav";
char const * SOUND_HIT2_FILE = "Data/Audio/hit2.wav";
char const * SOUND_HIT3_FILE = "Data/Audio/hit3.wav";
size_t const SOUND_HIT_COUNT = 3;
int const LAYER_BACKGROUND = -1000;
int const LAYER_FX = -500;
int const LAYER_CRYSTAL = -5;
int const LAYER_PLAYER = 0;
int const LAYER_WEAPON = 5;
int const LAYER_DEBUG_PLAYER = 10;
int const LAYER_EMOTE = 100;
int const LAYER_UI_GAME = 500;
int const LAYER_UI_FOREGROUND = 1000;
float const RADIUS_TO_SCALE = 2.f;



//-------------------------------------------------------------------------------------------------
Color GetColorFromIndex( byte_t netIndex )
{
	byte_t colorIndex = netIndex % 13;
	switch( colorIndex )
	{
	case 0:
		return Color::RED;
	case 1:
		return Color::ORANGE;
	case 2:
		return Color::YELLOW;
	case 3:
		return Color::GREEN;
	case 4:
		return Color::BLUE;
	case 5:
		return Color::PURPLE;
	case 6:
		return Color::MAGENTA;
	case 7:
		return Color::BROWN;
	case 8:
		return Color::CYAN;
	case 9:
		return Color::TURQUOISE;
	case 10:
		return Color::WHITE;
	case 11:
		return Color( 210, 180, 140, 255 );
	case 12:
		return Color::LIGHT_BLUE;
	default:
		return Color::BLACK;
	}
}


//-------------------------------------------------------------------------------------------------
Color GetColorFromClass( ePlayerClass const & classIndex )
{
	switch( classIndex )
	{
	case ePlayerClass_FIGHTER:
		return Color::LIGHT_RED;
	case ePlayerClass_WIZARD:
		return Color::LIGHT_BLUE;
	case ePlayerClass_ROGUE:
		return Color::LIGHT_GREEN;
	default:
		return Color::BLACK;
	}
}