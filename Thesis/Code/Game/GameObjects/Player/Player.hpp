#pragma once

#include "Engine/RenderSystem/Color.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Game/General/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
class Color;
class Item;
class NetGameObject;
class NetMessage;
class NetSender;
class PlayerShip;
class UIContainer;
class UIItem;
struct XMLNode;
enum eGameEvent : byte_t;
enum eItemType : byte_t;


//-------------------------------------------------------------------------------------------------
enum ePlayerState
{
	ePlayerState_ENTER_GAME,
	ePlayerState_ALIVE,
	ePlayerState_DEAD,
};


//-------------------------------------------------------------------------------------------------
enum ePlayerClass : uint8_t
{
	ePlayerClass_FIGHTER,
	ePlayerClass_WIZARD,
	ePlayerClass_ROGUE,
	ePlayerClass_CULTIST,
	ePlayerClass_CTHULHU,
	ePlayerClass_COUNT,
	ePlayerClass_NONE,
};


//-------------------------------------------------------------------------------------------------
enum eStat
{
	eStat_HEALTH,
	eStat_SHIELD,
	eStat_ENERGY,
	eStat_AGILITY,
	eStat_INVENTORY_SLOTS,
	eStat_COUNT,
};


//-------------------------------------------------------------------------------------------------
enum eEquipmentSlot
{
	eEquipmentSlot_PRIMARY,
	eEquipmentSlot_SECONDARY,
	eEquipmentSlot_COUNT,
};


//-------------------------------------------------------------------------------------------------
class Player
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static size_t const MAX_USERNAME_SIZE = 32;
	static char const * UI_INVENTORY;
	static byte_t const MAX_INVENTORY_SIZE = 12;
	static size_t const NO_PASSWORD = 0;
	static float const EJECT_DISTANCE;
	static uint16_t const MAX_MONEY;
	static byte_t const CHOOSE_CLASS_LEVEL;
	static byte_t const MAX_LEVEL;
	static uint16_t const CUBE_COST_PER_LEVEL;
	static float const BASE_STAT_STARTING_PERCENT;
	static float const BASE_STAT_GAIN_PERCENT_PER_LEVEL;
	static float const MAX_BONUS_STAT_GAIN_PERCENT_PER_LEVEL;
	static uint16_t const MAX_HEALTH_INCREMENT;
	static uint16_t const MAX_SHIELD_INCREMENT;
	static uint16_t const MAX_ENERGY_INCREMENT;
	//[0:Max Health][1:Max Shields][2:Max Energy][3:Acceleration][4:Max Inventory]
	static uint16_t CLASS_MAX_STATS[ePlayerClass_COUNT][eStat_COUNT];
	//not actually CONST because of how I want to initialize it, but basically const

	static char const * PLAYER_DIRECTORY;
	static char const * PLAYER_FILE_PATTERN;
	static char const * STRING_PLAYER_DATA;
	static char const * STRING_USERNAME;
	static char const * STRING_PASSWORD;
	static char const * STRING_LOAD_FROM_FILE;
	static char const * STRING_LAST_POSITION;
	static char const * STRING_LAST_ROTATION;
	static char const * STRING_LAST_HEALTH;
	static char const * STRING_MONEY;
	static char const * STRING_LEVEL;
	static char const * STRING_CLASS;
	static char const * STRING_STAT_HEALTH;
	static char const * STRING_STAT_SHIELD;
	static char const * STRING_STAT_ENERGY;
	static char const * STRING_PRIMARY_WEAPON;
	static char const * STRING_INVENTORY;
	static char const * STRING_VALUE;
	static char const * STRING_ITEM;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void Initialize( );
	static bool ExistsButWrongPassword( char const * checkUsername, size_t checkPassword );
	static bool CorrectPasswordOrDoesNotExist( char const * checkUsername, size_t checkPassword );
	static std::string GetSpriteIDForClass( ePlayerClass playerClass );
	static int GetClassStatMax( ePlayerClass playerClass, eStat stat );
	static std::string ParseClassName( ePlayerClass playerClass );
	static Color ParseClassColor( ePlayerClass playerClass );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	char m_username[MAX_USERNAME_SIZE];
	Color m_color;
	ePlayerState m_state;
	size_t m_password;
	byte_t m_netIndex;
	bool m_spawnFromLoad;
	Vector2f m_loadPosition;
	float m_loadRotation;
	uint16_t m_loadHealth;

//Synced Members
public:
	uint16_t m_money; //0
	uint16_t m_lastMoney;
	uint8_t m_level; //1 : level 0 means choose class
	uint8_t m_lastLevel;
	ePlayerClass m_class; //2
	ePlayerClass m_lastClass;
	uint16_t m_currentHealth; //3
	uint16_t m_lastHealth;
	uint16_t m_currentShield; //4
	uint16_t m_lastShield;
	uint16_t m_currentEnergy; //5
	uint16_t m_lastEnergy;
	uint8_t m_inventorySize; //7
	uint8_t m_lastInventorySize;
	Item * m_inventory[MAX_INVENTORY_SIZE];
	Item * m_equipment[eEquipmentSlot_COUNT]; //6
	uint16_t m_lastPrimaryEquipment;
	bool m_forceUpdate;

// Non-Synced Members
public:
	UIContainer * m_inventoryContainer;
	UIContainer * m_equipmentContainer[eEquipmentSlot_COUNT];
	NetGameObject * m_localShip;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	void WriteToMessage( NetMessage * out_message );
	void UpdateFromMessage( NetSender const & sender, NetMessage const & message );
	
	void WriteToXMLNode( XMLNode * out_playerData );
	void HostLoadPlayerFromFile( );
	
	Player( byte_t netIndex, char const * username, size_t password );
	~Player( );


	void HostAddLevel( );
	void HostAddHealth( );
	void HostAddShield( );
	void HostAddEnergy( );
	bool HostAddItemToInventory( uint16_t itemCode );
	void HostRemoveItemFromInventory( byte_t itemIndex );
	void HostEjectItemFromInventory( byte_t itemIndex );
	void HostEquipItem( eEquipmentSlot const & slot, byte_t itemIndex );
	void HostRemoveEquipment( eEquipmentSlot const & slot );
	void HostResetPlayer( );
	void HostClearInventory( );
	void HostTurnIntoCthulhu( );
	void ClientSetEquipment( eEquipmentSlot const & slot, uint16_t itemCode );
	void ClientAddOrChangeItemToInventory( byte_t inventoryIndex, uint16_t itemType );
	void ClientRemoveItemFromInventory( byte_t inventoryIndex );

	void CheckRespawn( );
	void SetInputState( uint16_t inputBitField );
	void ApplyInputEvent( eGameEvent const & inputEvent );
	void HostSetStartingStats( ePlayerClass const & playerClass );

	uint8_t CalculateDeltaBitfield( );
	uint16_t CalculateInventoryBitfield( ) const;
	uint16_t GetInputBitfield( ) const;
	char const * GetUsername( ) const;
	size_t GetPassword( ) const;
	Color GetColor( ) const;
	PlayerShip * GetPlayerShip( ) const;
	byte_t GetPlayerIndex( ) const;
	uint16_t GetNextUpgradeCost( ) const;
	bool GetInventoryIndexForItem( UIItem * findItem, byte_t * out_index ) const;
	bool GetEquipmentSlotForItem( UIItem * findItem, eEquipmentSlot * out_slot ) const;
	byte_t GetLastInventoryIndex( ) const;
	uint16_t GetEquippedWeaponCode( ) const;
	Vector2f GetLastPosition( ) const;
	bool HasShip( ) const;
	bool CanUpgradeHull( ) const;
	bool CanUpgradeHealth( ) const;
	bool CanUpgradeShield( ) const;
	bool CanUpgradeEnergy( ) const;
	bool CanChooseClass( ) const;
	bool ShouldSpawnFromLoad( ) const;
	bool IsMaxLevel( ) const;
	bool IsHealthStatMaxed( ) const;
	bool IsShieldStatMaxed( ) const;
	bool IsEnergyStatMaxed( ) const;
	bool IsLocalClient( ) const;
	bool IsEvilClass( ) const;

	void LoadLastData( PlayerShip * ship );
	void SetLastPosition( );
};