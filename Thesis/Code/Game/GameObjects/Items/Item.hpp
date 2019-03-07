#pragma once
#include "Game/General/GameCommon.hpp"
#include <vector>


//-------------------------------------------------------------------------------------------------
enum eItemType : uint8_t
{
	eItemType_ITEM_POTION,
	eItemType_CUBES,
	eItemType_WEAPON_PHASER,	//Weapons start
	eItemType_WEAPON_ROCKET,
	eItemType_WEAPON_ACID,
	eItemType_WEAPON_ZAPPER,
	eItemType_WEAPON_BOMBER,
	eItemType_WEAPON_FREEZE,	//Weapon ends
	eItemType_ALTERNATE_SCANNER,
	eItemType_ALTERNATE_REPULSOR,
	eItemType_ALTERNATE_SMOKE,
	eItemType_ALTERNATE_CONVERTER,
	eItemType_ALTERNATE_TELEPORT,
	eItemType_ALTERNATE_FAKE,
	eItemType_COUNT,
	
	eItemType_WEAPON_DRILL, //Temporarily removing drill
	
	eItemType_ERROR = 255,
};


//-------------------------------------------------------------------------------------------------
class UIContainer;
class UIItem;


//-------------------------------------------------------------------------------------------------
class Item
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static uint16_t const INVALID_ITEM_CODE;
	static uint8_t const STAT_DAMAGE = 0;
	static uint8_t const STAT_COST = 1;
	static uint8_t const STAT_RATE = 2;
	static uint8_t const STAT_SIZE = 3;
	static std::vector<std::vector<uint8_t>> const STAT_SETS;
	static std::vector<float> const FIRE_RATES;
	static std::vector<uint8_t> const ENERGY_COST;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static uint16_t GenerateRandomWeapon( byte_t level );
	static uint16_t GenerateRandomItem( eItemType const & type );
	static uint16_t GenerateRandomItem( eItemType const & type, byte_t level );
	static uint16_t GetItemCode( eItemType const & type, byte_t level, byte_t seed );
	static std::string ParseSprite( uint16_t itemCode );
	static std::string ParseIconSprite( uint16_t itemCode );
	static std::string ParseBulletSprite( uint16_t itemCode );

	static eItemType ParseItemType( uint16_t itemCode );
	static byte_t ParseLevel( uint16_t itemCode );
	static byte_t ParseSeed( uint16_t itemCode );
	static uint16_t ParseDamage( uint16_t itemCode );
	static uint16_t ParseEnergyCost( uint16_t itemCode );
	static float ParseFireRate( uint16_t itemCode );
	static uint8_t ParseSizeAttribute( uint16_t itemCode );
	static uint8_t ParseCubeAmount( uint16_t itemCode );
	static void ParseItemCode( uint16_t itemCode, eItemType * out_type, byte_t * out_level, byte_t * out_seed );
	static std::string ParseName( uint16_t itemCode );
	static std::string ParseEffectString( uint16_t itemCode );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	//eItemType m_itemType;	// 8 Bits (256 values)
	//byte_t m_itemLevel;	// 4 Bits (16 values)
	//byte_t m_itemSeed;	// 4 Bits (16 values)
	uint16_t m_itemCode;	// 16 Bits
	UIItem * m_boundUIItem;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Item( uint16_t itemCode );
	~Item( );

	void AttachToUIItem( UIContainer * container );
	void DetachFromUIItem( );

	eItemType GetItemType( ) const;
	bool IsWeapon( ) const;
	std::string GetSprite( ) const;
	std::string GetIconSprite( ) const;
	std::string GetBulletSprite( ) const;
};