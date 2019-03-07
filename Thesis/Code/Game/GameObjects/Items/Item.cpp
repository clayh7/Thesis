#include "Game/GameObjects/Items/Item.hpp"

#include "Engine/UISystem/UIContainer.hpp"
#include "Engine/UISystem/UIItem.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC uint16_t const Item::INVALID_ITEM_CODE = 0xFFFF;

//This converts 0-15 into a set of stats. Size = 16
//Damage | Cost | Fire Rate | Attribute
STATIC std::vector<std::vector<byte_t>> const Item::STAT_SETS =
{
	{ 1, 2, 4, 3 },
	{ 1, 3, 2, 4 },
	{ 1, 3, 4, 2 },
	{ 1, 4, 2, 3 },
	{ 2, 1, 4, 3 },
	{ 2, 3, 1, 4 },
	{ 2, 3, 4, 1 },
	{ 2, 4, 1, 3 },
	{ 3, 1, 4, 2 },
	{ 3, 2, 1, 4 },
	{ 3, 2, 4, 1 },
	{ 3, 4, 1, 2 },
	{ 4, 1, 3, 2 },
	{ 4, 2, 1, 3 },
	{ 4, 2, 3, 1 },
	{ 4, 3, 1, 2 }
};
STATIC std::vector<float> const Item::FIRE_RATES =
{
	1.0f, 0.66f, 0.5f, 0.33f
};
STATIC std::vector<uint8_t> const Item::ENERGY_COST =
{
	9U, 7U, 5U, 3U
};


//-------------------------------------------------------------------------------------------------
STATIC uint16_t Item::GenerateRandomWeapon( byte_t level )
{
	eItemType itemType = (eItemType) RandomInt( (int) eItemType_WEAPON_PHASER, (int) eItemType_WEAPON_FREEZE + 1 );
	return GenerateRandomItem( itemType, level );
}


//-------------------------------------------------------------------------------------------------
STATIC uint16_t Item::GenerateRandomItem( eItemType const & type )
{
	byte_t level = (byte_t) ( 1 + RandomInt( 10 ) );
	return GenerateRandomItem( type, level );
}


//-------------------------------------------------------------------------------------------------
STATIC uint16_t Item::GenerateRandomItem( eItemType const & type, byte_t level )
{
	byte_t seed = (byte_t) RandomInt( 16 );
	return GetItemCode( type, level, seed );
}


//-------------------------------------------------------------------------------------------------
STATIC uint16_t Item::GetItemCode( eItemType const & type, byte_t level, byte_t seed )
{
	uint16_t code = 0;
	byte_t bitMask = 0b00001111;
	byte_t levelBits = level & bitMask;
	byte_t seedBits = seed & bitMask;
	code = code | type; // 8 bits
	code = code << 4;
	code = code | levelBits; // 4 bits
	code = code << 4;
	code = code | seedBits; // 4 bits
	return code;
}


//-------------------------------------------------------------------------------------------------
STATIC eItemType Item::ParseItemType( uint16_t itemCode )
{
	itemCode = itemCode >> 8;
	return (eItemType) itemCode;
}


//-------------------------------------------------------------------------------------------------
STATIC byte_t Item::ParseLevel( uint16_t itemCode )
{
	byte_t bitMask4 = 0b00001111;
	itemCode = itemCode >> 4;
	itemCode = itemCode & bitMask4;
	return (byte_t)itemCode;
}


//-------------------------------------------------------------------------------------------------
STATIC byte_t Item::ParseSeed( uint16_t itemCode )
{
	byte_t bitMask4 = 0b00001111;
	itemCode = itemCode & bitMask4;
	return (byte_t)itemCode;
}


//-------------------------------------------------------------------------------------------------
STATIC uint16_t Item::ParseDamage( uint16_t itemCode )
{
	eItemType type;
	uint8_t level;
	uint8_t seed;
	Item::ParseItemCode( itemCode, &type, &level, &seed );
	uint8_t damageStat = STAT_SETS[seed][STAT_DAMAGE];
	uint16_t damage = 4 * ( damageStat + level - 1 );
	if( type == eItemType_WEAPON_BOMBER )
	{
		return damage * 2U;
	}
	return damage;
}


//-------------------------------------------------------------------------------------------------
STATIC uint16_t Item::ParseEnergyCost( uint16_t itemCode )
{
	uint8_t seed = Item::ParseSeed( itemCode );
	uint8_t energyCostStat = STAT_SETS[seed][STAT_COST];
	return ENERGY_COST[energyCostStat - 1];
}


//-------------------------------------------------------------------------------------------------
STATIC float Item::ParseFireRate( uint16_t itemCode )
{
	uint8_t seed = Item::ParseSeed( itemCode );
	uint8_t fireRateStat = STAT_SETS[seed][STAT_RATE];
	return FIRE_RATES[fireRateStat - 1];
}


//-------------------------------------------------------------------------------------------------
STATIC uint8_t Item::ParseSizeAttribute( uint16_t itemCode )
{
	uint8_t seed = Item::ParseSeed( itemCode );
	return STAT_SETS[seed][STAT_SIZE];
}


//-------------------------------------------------------------------------------------------------
STATIC uint8_t Item::ParseCubeAmount( uint16_t itemCode )
{
	eItemType type = ParseItemType( itemCode );
	if( type == eItemType_CUBES )
	{
		uint16_t byteMask = 0x00FF;
		itemCode = itemCode & byteMask;
		return (uint8_t) itemCode;
	}
	else
	{
		return 0U;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void Item::ParseItemCode( uint16_t itemCode, eItemType * out_type, byte_t * out_level, byte_t * out_seed )
{
	byte_t bitMask4 = 0b00001111;
	*out_seed = itemCode & bitMask4;
	itemCode = itemCode >> 4;
	*out_level = itemCode & bitMask4;
	itemCode = itemCode >> 4;
	*out_type = (eItemType) itemCode;
}


//-------------------------------------------------------------------------------------------------
STATIC std::string Item::ParseName( uint16_t itemCode )
{
	eItemType itemType = ParseItemType( itemCode );
	switch( itemType )
	{
	case eItemType_ITEM_POTION:
		return "Repair Bot";
	case eItemType_CUBES:
		return "Cubes";
	case eItemType_WEAPON_PHASER:
		return "Phasers";
	case eItemType_WEAPON_ROCKET:
		return "Rocket Launcher";
	case eItemType_WEAPON_DRILL:
		return "Drill Mod";
	case eItemType_WEAPON_ACID:
		return "Acid Shooter";
	case eItemType_WEAPON_ZAPPER:
		return "Zap Cannon";
	case eItemType_WEAPON_BOMBER:
		return "Bomber";
	case eItemType_WEAPON_FREEZE:
		return "Freeze Gun";
	case eItemType_ALTERNATE_SCANNER:
	case eItemType_ALTERNATE_REPULSOR:
	case eItemType_ALTERNATE_SMOKE:
	case eItemType_ALTERNATE_CONVERTER:
	case eItemType_ALTERNATE_TELEPORT:
	case eItemType_ALTERNATE_FAKE:
	case eItemType_ERROR:
		return "INVALID";
	}
	return "INVALID";
}


//-------------------------------------------------------------------------------------------------
STATIC std::string Item::ParseEffectString( uint16_t itemCode )
{
	eItemType itemType = ParseItemType( itemCode );
	uint8_t itemSize = ParseSizeAttribute( itemCode );
	switch( itemType )
	{
	case eItemType_WEAPON_PHASER:
		return Stringf( "Shoots %u bullet%s", itemSize, itemSize == 1 ? " " : "s" );
	case eItemType_WEAPON_ROCKET:
		return Stringf( "Area splash size: %u", itemSize );
	case eItemType_WEAPON_ACID:
		return itemSize == 1 ? "No effect" : Stringf( "%ux damage vs hull", itemSize );
	case eItemType_WEAPON_ZAPPER:
		return itemSize == 1 ? "No effect" : Stringf( "%ux damage vs shields", itemSize );
	case eItemType_WEAPON_BOMBER:
		return Stringf( "Area splash size: %u", itemSize );
	case eItemType_WEAPON_FREEZE:
		return Stringf( "Slow percentage %u%%", itemSize * 15U );
	case eItemType_ITEM_POTION:
	case eItemType_CUBES:
		return Stringf( "Stack of %u cubes.", ParseCubeAmount( itemCode ) );
	case eItemType_WEAPON_DRILL:
	case eItemType_ALTERNATE_SCANNER:
	case eItemType_ALTERNATE_REPULSOR:
	case eItemType_ALTERNATE_SMOKE:
	case eItemType_ALTERNATE_CONVERTER:
	case eItemType_ALTERNATE_TELEPORT:
	case eItemType_ALTERNATE_FAKE:
	case eItemType_ERROR:
		return " ";
	}
	return " ";
}


//-------------------------------------------------------------------------------------------------
STATIC std::string Item::ParseSprite( uint16_t itemCode )
{
	eItemType itemType = ParseItemType( itemCode );
	switch( itemType )
	{
	case eItemType_ITEM_POTION:
		return "heart";
	case eItemType_CUBES:
		return "cubes";
	case eItemType_WEAPON_PHASER:
		return "weaponPhaser";
	case eItemType_WEAPON_ROCKET:
		return "weaponRocket";
	case eItemType_WEAPON_DRILL:
		return "weaponDrill";
	case eItemType_WEAPON_ACID:
		return "weaponAcid";
	case eItemType_WEAPON_ZAPPER:
		return "weaponZapper";
	case eItemType_WEAPON_BOMBER:
		return "weaponBomber";
	case eItemType_WEAPON_FREEZE:
		return "weaponFreeze";
	case eItemType_ALTERNATE_SCANNER:
	case eItemType_ALTERNATE_REPULSOR:
	case eItemType_ALTERNATE_SMOKE:
	case eItemType_ALTERNATE_CONVERTER:
	case eItemType_ALTERNATE_TELEPORT:
	case eItemType_ALTERNATE_FAKE:
	case eItemType_ERROR:
		return "error";
	}
	return "NEED TO ADD SPRITE ID FOR ITEM TYPE";
}


//-------------------------------------------------------------------------------------------------
STATIC std::string Item::ParseIconSprite( uint16_t itemCode )
{
	eItemType itemType = ParseItemType( itemCode );
	switch( itemType )
	{
	case eItemType_ITEM_POTION:
		return "heart";
	case eItemType_CUBES:
		return "cubes";
	case eItemType_WEAPON_PHASER:
		return "weaponPhaserIcon";
	case eItemType_WEAPON_ROCKET:
		return "weaponRocketIcon";
	case eItemType_WEAPON_DRILL:
		return "weaponDrillIcon";
	case eItemType_WEAPON_ACID:
		return "weaponAcidIcon";
	case eItemType_WEAPON_ZAPPER:
		return "weaponZapperIcon";
	case eItemType_WEAPON_BOMBER:
		return "weaponBomberIcon";
	case eItemType_WEAPON_FREEZE:
		return "weaponFreezeIcon";
	case eItemType_ALTERNATE_SCANNER:
	case eItemType_ALTERNATE_REPULSOR:
	case eItemType_ALTERNATE_SMOKE:
	case eItemType_ALTERNATE_CONVERTER:
	case eItemType_ALTERNATE_TELEPORT:
	case eItemType_ALTERNATE_FAKE:
	case eItemType_ERROR:
		return "error";
	}
	return "NEED TO ADD SPRITE ID FOR ITEM TYPE";
}


//-------------------------------------------------------------------------------------------------
std::string Item::ParseBulletSprite( uint16_t itemCode )
{
	eItemType itemType = ParseItemType( itemCode );
	switch( itemType )
	{
	case eItemType_ITEM_POTION:
		return "heart";
	case eItemType_WEAPON_PHASER:
		return "bulletPhaser";
	case eItemType_WEAPON_ROCKET:
		return "bulletRocket";
	case eItemType_WEAPON_DRILL:
		return "bulletDrill";
	case eItemType_WEAPON_ACID:
		return "bulletAcid";
	case eItemType_WEAPON_ZAPPER:
		return "bulletZapper";
	case eItemType_WEAPON_BOMBER:
		return "bulletBomb";
	case eItemType_WEAPON_FREEZE:
		return "bulletFreeze";
	case eItemType_ALTERNATE_SCANNER:
	case eItemType_ALTERNATE_REPULSOR:
	case eItemType_ALTERNATE_SMOKE:
	case eItemType_ALTERNATE_CONVERTER:
	case eItemType_ALTERNATE_TELEPORT:
	case eItemType_ALTERNATE_FAKE:
	case eItemType_ERROR:
		return "error";
	}
	return "NEED TO ADD SPRITE ID FOR ITEM TYPE";
}


//-------------------------------------------------------------------------------------------------
Item::Item( uint16_t itemCode )
	: m_itemCode( itemCode )
	, m_boundUIItem( nullptr )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Item::~Item( )
{
	DetachFromUIItem( );
}

//-------------------------------------------------------------------------------------------------
void Item::AttachToUIItem( UIContainer * container )
{
	m_boundUIItem = g_UISystem->CreateWidget<UIItem>( );
	m_boundUIItem->SetProperty( UIItem::PROPERTY_SPRITE_ID, GetIconSprite( ) );
	container->AddChild( m_boundUIItem );
}


//-------------------------------------------------------------------------------------------------
void Item::DetachFromUIItem( )
{
	if( m_boundUIItem )
	{
		g_UISystem->RemoveWidget( m_boundUIItem );
		m_boundUIItem = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
eItemType Item::GetItemType( ) const
{
	return ParseItemType( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
bool Item::IsWeapon( ) const
{
	eItemType itemType = ParseItemType( m_itemCode );
	if( itemType >= eItemType_WEAPON_PHASER && itemType <= eItemType_WEAPON_FREEZE )
	{
		return true;
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
std::string Item::GetSprite( ) const
{
	return ParseSprite( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
std::string Item::GetIconSprite( ) const
{
	return ParseIconSprite( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
std::string Item::GetBulletSprite( ) const
{
	return ParseBulletSprite( m_itemCode );
}