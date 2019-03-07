#include "Game/GameObjects/Player/Player.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UIContainer.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/NetGameObject.hpp"
#include "Game/GameObjects/Other/Emote.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * Player::UI_INVENTORY = "playerInventory";
STATIC float const Player::EJECT_DISTANCE = -1.f;
STATIC uint16_t const Player::MAX_MONEY = 9999U;
STATIC byte_t const Player::CHOOSE_CLASS_LEVEL = 0U;
STATIC byte_t const Player::MAX_LEVEL = 10U;
STATIC uint16_t const Player::CUBE_COST_PER_LEVEL = 50U;
STATIC float const Player::BASE_STAT_STARTING_PERCENT = ( 1.f / 6.f );
STATIC float const Player::BASE_STAT_GAIN_PERCENT_PER_LEVEL = ( 1.f / 22.f );
STATIC float const Player::MAX_BONUS_STAT_GAIN_PERCENT_PER_LEVEL = ( 1.f / 22.f );
STATIC uint16_t const Player::MAX_HEALTH_INCREMENT = 20U;
STATIC uint16_t const Player::MAX_SHIELD_INCREMENT = 10U;
STATIC uint16_t const Player::MAX_ENERGY_INCREMENT = 5U;
STATIC uint16_t Player::CLASS_MAX_STATS[ePlayerClass_COUNT][eStat_COUNT];
STATIC char const * Player::PLAYER_DIRECTORY = "Data/Saves/Players/";
STATIC char const * Player::PLAYER_FILE_PATTERN = "*.Player.xml";
STATIC char const * Player::STRING_PLAYER_DATA = "PlayerData";
STATIC char const * Player::STRING_USERNAME = "username";
STATIC char const * Player::STRING_PASSWORD = "password";
STATIC char const * Player::STRING_LOAD_FROM_FILE = "loadFromFile";
STATIC char const * Player::STRING_LAST_POSITION = "lastPosition";
STATIC char const * Player::STRING_LAST_ROTATION = "lastRotation";
STATIC char const * Player::STRING_LAST_HEALTH = "lastHealth";
STATIC char const * Player::STRING_MONEY = "money";
STATIC char const * Player::STRING_LEVEL = "level";
STATIC char const * Player::STRING_CLASS = "class";
STATIC char const * Player::STRING_STAT_HEALTH = "statHP";
STATIC char const * Player::STRING_STAT_SHIELD = "statShield";
STATIC char const * Player::STRING_STAT_ENERGY = "statEnergy";
STATIC char const * Player::STRING_PRIMARY_WEAPON = "primaryWeapon";
STATIC char const * Player::STRING_INVENTORY = "inventory";
STATIC char const * Player::STRING_VALUE = "value";
STATIC char const * Player::STRING_ITEM = "item";


//-------------------------------------------------------------------------------------------------
STATIC void Player::Initialize( )
{
	//Fighter Stats
	CLASS_MAX_STATS[ePlayerClass_FIGHTER][eStat_HEALTH] = 1000U;
	CLASS_MAX_STATS[ePlayerClass_FIGHTER][eStat_SHIELD] = 150U;
	CLASS_MAX_STATS[ePlayerClass_FIGHTER][eStat_ENERGY] = 150U;
	CLASS_MAX_STATS[ePlayerClass_FIGHTER][eStat_AGILITY] = 400U;
	CLASS_MAX_STATS[ePlayerClass_FIGHTER][eStat_INVENTORY_SLOTS] = 4U;

	//Wizard Stats
	CLASS_MAX_STATS[ePlayerClass_WIZARD][eStat_HEALTH] = 600U;
	CLASS_MAX_STATS[ePlayerClass_WIZARD][eStat_SHIELD] = 400U;
	CLASS_MAX_STATS[ePlayerClass_WIZARD][eStat_ENERGY] = 300U;
	CLASS_MAX_STATS[ePlayerClass_WIZARD][eStat_AGILITY] = 225U;
	CLASS_MAX_STATS[ePlayerClass_WIZARD][eStat_INVENTORY_SLOTS] = 6U;

	//Rogue Stats
	CLASS_MAX_STATS[ePlayerClass_ROGUE][eStat_HEALTH] = 450U;
	CLASS_MAX_STATS[ePlayerClass_ROGUE][eStat_SHIELD] = 200U;
	CLASS_MAX_STATS[ePlayerClass_ROGUE][eStat_ENERGY] = 200U;
	CLASS_MAX_STATS[ePlayerClass_ROGUE][eStat_AGILITY] = 100U;
	CLASS_MAX_STATS[ePlayerClass_ROGUE][eStat_INVENTORY_SLOTS] = 9U;

	//Cultist Stats
	CLASS_MAX_STATS[ePlayerClass_CULTIST][eStat_HEALTH] = 900U;
	CLASS_MAX_STATS[ePlayerClass_CULTIST][eStat_SHIELD] = 300U;
	CLASS_MAX_STATS[ePlayerClass_CULTIST][eStat_ENERGY] = 250U;
	CLASS_MAX_STATS[ePlayerClass_CULTIST][eStat_AGILITY] = 225U;
	CLASS_MAX_STATS[ePlayerClass_CULTIST][eStat_INVENTORY_SLOTS] = 6U;
}


//-------------------------------------------------------------------------------------------------
STATIC bool Player::ExistsButWrongPassword( char const * checkUsername, size_t checkPassword )
{
	std::vector<std::string> playerSaveFiles = EnumerateFilesInFolder( PLAYER_DIRECTORY, PLAYER_FILE_PATTERN );

	//No save files found
	if( playerSaveFiles.size( ) == 0 )
	{
		//No save files
		return false;
	}

	//Find your file from username
	for( std::string & playerSavePath : playerSaveFiles )
	{
		XMLNode playerData = XMLNode::openFileHelper( playerSavePath.c_str( ) ).getChildNode( STRING_PLAYER_DATA );

		//If this is the correct player
		std::string username;
		username = ReadXMLAttribute( playerData, STRING_USERNAME, username );
		if( strncmp( username.c_str( ), checkUsername, MAX_USERNAME_SIZE ) == 0 )
		{
			size_t password = ReadXMLAttribute( playerData, STRING_PASSWORD, 0 );

			//Correct Password
			if( password == checkPassword ||
				password == NO_PASSWORD )
			{
				//Username + password match
				return false;
			}
			else
			{
				//Username is correct but passwords do not match
				return true;
			}
		}
	}

	//Player save doesn't exist, so player data is fine
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool Player::CorrectPasswordOrDoesNotExist( char const * checkUsername, size_t checkPassword )
{
	return !ExistsButWrongPassword( checkUsername, checkPassword );
}


//-------------------------------------------------------------------------------------------------
STATIC std::string Player::GetSpriteIDForClass( ePlayerClass playerClass )
{
	switch( playerClass )
	{
	case ePlayerClass_FIGHTER:
		return "shipFighter";
	case ePlayerClass_WIZARD:
		return "shipWizard";
	case ePlayerClass_ROGUE:
		return "shipRogue";
	case ePlayerClass_CULTIST:
		return "shipCultist";
	case ePlayerClass_CTHULHU:
		return "enemyBossCthulhu";
	default:
		return "error";
	}
}


//-------------------------------------------------------------------------------------------------
STATIC std::string Player::ParseClassName( ePlayerClass playerClass )
{
	switch( playerClass )
	{
	case ePlayerClass_FIGHTER:
		return "Fighter";
	case ePlayerClass_WIZARD:
		return "Wizard";
	case ePlayerClass_ROGUE:
		return "Rogue";
	case ePlayerClass_CULTIST:
		return "Cultist";
	case ePlayerClass_CTHULHU:
		return "Cthulhu";
	default:
		return "Error";
	}
}


//-------------------------------------------------------------------------------------------------
STATIC Color Player::ParseClassColor( ePlayerClass playerClass )
{
	switch( playerClass )
	{
	case ePlayerClass_FIGHTER:
		return Color::DARK_RED;
	case ePlayerClass_WIZARD:
		return Color::DARK_BLUE;
	case ePlayerClass_ROGUE:
		return Color::DARK_GREEN;
	case ePlayerClass_CULTIST:
		return Color::DARK_GREY;
	case ePlayerClass_CTHULHU:
		return Color::PURPLE;
	default:
		return Color::WHITE;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC int Player::GetClassStatMax( ePlayerClass playerClass, eStat stat )
{
	return (int) CLASS_MAX_STATS[playerClass][stat];
}


//-------------------------------------------------------------------------------------------------
STATIC void Player::WriteToMessage( NetMessage * out_message )
{
	//Write Stats
	uint8_t deltaBitfield = CalculateDeltaBitfield( );
	out_message->Write<uint8_t>( deltaBitfield );
	if( IsBitSet( deltaBitfield, 0 ) )
	{
		out_message->Write<uint16_t>( m_money );
	}
	if( IsBitSet( deltaBitfield, 1 ) )
	{
		out_message->Write<uint8_t>( m_level );
	}
	if( IsBitSet( deltaBitfield, 2 ) )
	{
		out_message->Write<ePlayerClass>( m_class );
	}
	if( IsBitSet( deltaBitfield, 3 ) )
	{
		out_message->Write<uint16_t>( m_currentHealth );
	}
	if( IsBitSet( deltaBitfield, 4 ) )
	{
		out_message->Write<uint16_t>( m_currentShield );
	}
	if( IsBitSet( deltaBitfield, 5 ) )
	{
		out_message->Write<uint16_t>( m_currentEnergy );
	}
	if( IsBitSet( deltaBitfield, 6 ) )
	{
		out_message->Write<uint16_t>( GetEquippedWeaponCode( ) );
	}
	if( IsBitSet( deltaBitfield, 7 ) )
	{
		//Write Inventory
		uint16_t inventoryBitfield = CalculateInventoryBitfield( );
		out_message->Write<uint16_t>( inventoryBitfield );
		for( byte_t inventoryIndex = 0; inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
		{
			if( IsBitSet( inventoryBitfield, inventoryIndex ) )
			{
				Item * currentItem = m_inventory[inventoryIndex];
				out_message->Write<uint16_t>( currentItem->m_itemCode );
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Player::UpdateFromMessage( NetSender const &, NetMessage const & message )
{
	//Read Stats
	bool levelUp = false;
	uint16_t primaryWeaponItemCode = Item::INVALID_ITEM_CODE;
	uint8_t deltaBitfield = 0U;

	message.Read<uint8_t>( &deltaBitfield );
	if( IsBitSet( deltaBitfield, 0 ) )
	{
		message.Read<uint16_t>( &m_money );
	}
	if( IsBitSet( deltaBitfield, 1 ) )
	{
		uint8_t previousLevel = m_level;
		message.Read<uint8_t>( &m_level );
		if( m_level > previousLevel )
		{
			levelUp = true;
		}
	}
	if( IsBitSet( deltaBitfield, 2 ) )
	{
		message.Read<ePlayerClass>( &m_class );
	}
	if( IsBitSet( deltaBitfield, 3 ) )
	{
		uint16_t previousHealth = m_currentHealth;
		message.Read<uint16_t>( &m_currentHealth );
		if( m_currentHealth > previousHealth )
		{
			levelUp = true;
		}
	}
	if( IsBitSet( deltaBitfield, 4 ) )
	{
		uint16_t previousShield = m_currentShield;
		message.Read<uint16_t>( &m_currentShield );
		if( m_currentShield > previousShield )
		{
			levelUp = true;
		}
	}
	if( IsBitSet( deltaBitfield, 5 ) )
	{
		uint16_t previousEnergy = m_currentEnergy;
		message.Read<uint16_t>( &m_currentEnergy );
		if( m_currentEnergy > previousEnergy )
		{
			levelUp = true;
		}
	}
	if( IsBitSet( deltaBitfield, 6 ) )
	{
		message.Read<uint16_t>( &primaryWeaponItemCode );
		ClientSetEquipment( eEquipmentSlot_PRIMARY, primaryWeaponItemCode );
	}
	if( IsBitSet( deltaBitfield, 7 ) )
	{
		//Read Inventory
		uint16_t inventoryBitfield = 0U;
		message.Read<uint16_t>( &inventoryBitfield );
		uint8_t inventorySize = 0U;
		for( uint8_t currentInventoryIndex = 0; currentInventoryIndex < MAX_INVENTORY_SIZE; ++currentInventoryIndex )
		{
			if( IsBitSet( inventoryBitfield, currentInventoryIndex ) )
			{
				uint16_t itemCode;
				message.Read<uint16_t>( &itemCode );
				ClientAddOrChangeItemToInventory( currentInventoryIndex, itemCode );
				inventorySize++;
			}
			else
			{
				ClientRemoveItemFromInventory( currentInventoryIndex );
			}
		}
		m_inventorySize = inventorySize;
	}


	//Show level up effect
	if( levelUp )
	{
		g_GameSystem->ClientLevelUpEffect( this );
	}
}


//-------------------------------------------------------------------------------------------------
void Player::WriteToXMLNode( XMLNode * out_playerData )
{
	//#TODO: Change all strings to variables
	XMLNode child;
	out_playerData->addAttribute( STRING_USERNAME, GetUsername( ) );
	out_playerData->addAttribute( STRING_PASSWORD, Stringf( "%u", m_password ).c_str( ) );
	PlayerShip * ship = GetPlayerShip( );
	if( ship )
	{
		out_playerData->addAttribute( STRING_LOAD_FROM_FILE, "true" );
		out_playerData->addAttribute( STRING_LAST_POSITION, Stringf( "%f,%f", ship->m_position.x, ship->m_position.y ).c_str( ) );
		out_playerData->addAttribute( STRING_LAST_ROTATION, Stringf( "%f", ship->m_rotationDegrees ).c_str( ) );
		out_playerData->addAttribute( STRING_LAST_HEALTH, Stringf( "%d", ship->m_health ).c_str( ) );
	}
	child = out_playerData->addChild( STRING_MONEY );
	child.addAttribute( STRING_VALUE, Stringf( "%u", m_money ).c_str( ) );
	child = out_playerData->addChild( STRING_LEVEL );
	child.addAttribute( STRING_VALUE, Stringf( "%u", m_level ).c_str( ) );
	child = out_playerData->addChild( STRING_CLASS );
	child.addAttribute( STRING_VALUE, Stringf( "%u", m_class ).c_str( ) );
	child = out_playerData->addChild( STRING_STAT_HEALTH );
	child.addAttribute( STRING_VALUE, Stringf( "%u", m_currentHealth ).c_str( ) );
	child = out_playerData->addChild( STRING_STAT_SHIELD );
	child.addAttribute( STRING_VALUE, Stringf( "%u", m_currentShield ).c_str( ) );
	child = out_playerData->addChild( STRING_STAT_ENERGY );
	child.addAttribute( STRING_VALUE, Stringf( "%u", m_currentEnergy ).c_str( ) );
	child = out_playerData->addChild( STRING_PRIMARY_WEAPON );
	child.addAttribute( STRING_VALUE, Stringf( "%u", GetEquippedWeaponCode( ) ).c_str( ) );

	XMLNode inventroyChild = out_playerData->addChild( STRING_INVENTORY );
	for( uint8_t inventoryIndex = 0U; inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
	{
		if( m_inventory[inventoryIndex] )
		{
			child = inventroyChild.addChild( STRING_ITEM );
			child.addAttribute( STRING_VALUE, Stringf( "%u", m_inventory[inventoryIndex]->m_itemCode ).c_str( ) );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Player::HostLoadPlayerFromFile( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	std::vector<std::string> playerSaveFiles = EnumerateFilesInFolder( PLAYER_DIRECTORY, PLAYER_FILE_PATTERN );

	//No save files found
	if( playerSaveFiles.size( ) == 0 )
	{
		return;
	}

	//Find your file from username
	for( std::string & playerSavePath : playerSaveFiles )
	{
		XMLNode playerData = XMLNode::openFileHelper( playerSavePath.c_str( ) ).getChildNode( STRING_PLAYER_DATA );
		size_t password = ReadXMLAttribute( playerData, STRING_PASSWORD, 0U );

		//Not Correct Password
		if( password != m_password )
		{
			continue;
		}

		//If this is the correct player
		std::string username;
		username = ReadXMLAttribute( playerData, STRING_USERNAME, username );
		if( strncmp( username.c_str( ), m_username, MAX_USERNAME_SIZE ) == 0 )
		{
			m_spawnFromLoad = ReadXMLAttribute( playerData, STRING_LOAD_FROM_FILE, false );
			m_loadPosition = ReadXMLAttribute( playerData, STRING_LAST_POSITION, m_loadPosition );
			m_loadRotation = ReadXMLAttribute( playerData, STRING_LAST_ROTATION, m_loadRotation );
			m_loadHealth = ReadXMLAttribute( playerData, STRING_LAST_HEALTH, m_loadHealth );
			m_money = ReadXMLAttribute( playerData, STRING_MONEY, m_money );
			m_level = ReadXMLAttribute( playerData, STRING_LEVEL, m_level );
			m_class = (ePlayerClass) ReadXMLAttribute( playerData, STRING_CLASS, (byte_t) m_class );
			m_currentHealth = ReadXMLAttribute( playerData, STRING_STAT_HEALTH, m_currentHealth );
			m_currentShield = ReadXMLAttribute( playerData, STRING_STAT_SHIELD, m_currentShield );
			m_currentEnergy = ReadXMLAttribute( playerData, STRING_STAT_ENERGY, m_currentEnergy );
			uint16_t primaryWeaponItemCode = ReadXMLAttribute( playerData, STRING_PRIMARY_WEAPON, Item::INVALID_ITEM_CODE );
			if( primaryWeaponItemCode != Item::INVALID_ITEM_CODE )
			{
				m_equipment[eEquipmentSlot_PRIMARY] = new Item( primaryWeaponItemCode );
			}
			XMLNode playerInventory = playerData.getChildNode( STRING_INVENTORY );
			for( int inventoryIndex = 0; inventoryIndex < playerInventory.nChildNode( ) && inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
			{
				uint16_t itemCode = ReadXMLAttribute( playerInventory.getChildNode( inventoryIndex ), STRING_VALUE, (uint16_t) 0U );
				HostAddItemToInventory( itemCode );
			}
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
Player::Player( byte_t netIndex, char const * username, size_t password )
	: m_color( GetColorFromIndex( netIndex ) )
	, m_state( ePlayerState_ENTER_GAME )
	, m_password( password )
	, m_netIndex( netIndex )
	, m_spawnFromLoad( false )
	, m_loadPosition( 0.f )
	, m_loadRotation( 0.f )
	, m_loadHealth( 1U )
	, m_money( 0U )
	, m_lastMoney( 0U )
	, m_level( 0U )
	, m_lastLevel( 0U )
	, m_class( ePlayerClass_NONE )
	, m_lastClass( ePlayerClass_NONE )
	, m_currentHealth( 0U )
	, m_lastHealth( 0U )
	, m_currentShield( 0U )
	, m_lastShield( 0U )
	, m_currentEnergy( 0U )
	, m_lastEnergy( 0U )
	, m_inventorySize( 0U )
	, m_lastInventorySize( 0U )
	, m_lastPrimaryEquipment( 0U )
	, m_forceUpdate( true )
	, m_inventoryContainer( nullptr )
	, m_localShip( nullptr )
{
	// if it was MAX_USERNAME_SIZE + 1 it would go over bounds, but we need that null
	size_t stringLength = strnlen_s( username, MAX_USERNAME_SIZE - 1 ) + 1;
	memcpy( m_username, username, stringLength );

	//Clear inventory
	for( byte_t inventoryIndex = 0; inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
	{
		m_inventory[inventoryIndex] = nullptr;
	}
	for( byte_t equipmentIndex = 0; equipmentIndex < eEquipmentSlot_COUNT; ++equipmentIndex )
	{
		m_equipment[equipmentIndex] = nullptr;
	}

	//Link inventory to UIContainer
	m_inventoryContainer = g_UISystem->GetWidgetByName<UIContainer>( UI_INVENTORY );
	m_equipmentContainer[eEquipmentSlot_PRIMARY] = g_UISystem->GetWidgetByName<UIContainer>( Game::UI_EQUIPMENT_PRIMARY );
	m_equipmentContainer[eEquipmentSlot_SECONDARY] = nullptr;
}


//-------------------------------------------------------------------------------------------------
Player::~Player( )
{
	for( byte_t inventoryIndex = 0; inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
	{
		if( m_inventory[inventoryIndex] )
		{
			delete m_inventory[inventoryIndex];
		}
		m_inventory[inventoryIndex] = nullptr;
	}

	for( byte_t equipmentIndex = 0; equipmentIndex < eEquipmentSlot_COUNT; ++equipmentIndex )
	{
		if( m_equipment[equipmentIndex] )
		{
			delete m_equipment[equipmentIndex];
		}
		m_equipment[equipmentIndex] = nullptr;
	}

	m_inventorySize = 0;
}


//-------------------------------------------------------------------------------------------------
void Player::HostAddLevel( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	m_money -= GetNextUpgradeCost( );
	m_level += 1;

	PlayerShip * currentShip = GetPlayerShip( );
	uint16_t currentDamage = 0U;
	if( currentShip != nullptr )
	{
		currentDamage = m_currentHealth - currentShip->m_health;
	}

	//Base stat increase
	m_currentHealth += (uint16_t) ( (float) CLASS_MAX_STATS[m_class][eStat_HEALTH] * BASE_STAT_GAIN_PERCENT_PER_LEVEL );
	m_currentShield += (uint16_t) ( (float) CLASS_MAX_STATS[m_class][eStat_SHIELD] * BASE_STAT_GAIN_PERCENT_PER_LEVEL );
	m_currentEnergy += (uint16_t) ( (float) CLASS_MAX_STATS[m_class][eStat_ENERGY] * BASE_STAT_GAIN_PERCENT_PER_LEVEL );

	uint16_t maxBonusHealth = (uint16_t) ( (float) CLASS_MAX_STATS[m_class][eStat_HEALTH] * MAX_BONUS_STAT_GAIN_PERCENT_PER_LEVEL );
	uint16_t maxBonusShield = (uint16_t) ( (float) CLASS_MAX_STATS[m_class][eStat_SHIELD] * MAX_BONUS_STAT_GAIN_PERCENT_PER_LEVEL );
	uint16_t maxBonusEnergy = (uint16_t) ( (float) CLASS_MAX_STATS[m_class][eStat_ENERGY] * MAX_BONUS_STAT_GAIN_PERCENT_PER_LEVEL );

	uint16_t bonusHealth = (uint16_t) RandomInt( (int) maxBonusHealth );
	uint16_t bonusShield = (uint16_t) RandomInt( (int) maxBonusShield );
	uint16_t bonusEnergy = (uint16_t) RandomInt( (int) maxBonusEnergy );

	//Random bonus stat increase
	m_currentHealth += bonusHealth;
	m_currentShield += bonusShield;
	m_currentEnergy += bonusEnergy;

	if( currentShip != nullptr )
	{
		currentShip->m_health = m_currentHealth - currentDamage;
	}
}


//-------------------------------------------------------------------------------------------------
void Player::HostAddHealth( )
{
	m_money -= GetNextUpgradeCost( );
	uint16_t maxHealth = CLASS_MAX_STATS[m_class][eStat_HEALTH];
	m_currentHealth += MAX_HEALTH_INCREMENT;
	m_currentHealth = Min( maxHealth, m_currentHealth );
}


//-------------------------------------------------------------------------------------------------
void Player::HostAddShield( )
{
	m_money -= GetNextUpgradeCost( );
	uint16_t maxShield = CLASS_MAX_STATS[m_class][eStat_SHIELD];
	m_currentShield += MAX_SHIELD_INCREMENT;
	m_currentShield = Min( maxShield, m_currentShield );
}


//-------------------------------------------------------------------------------------------------
void Player::HostAddEnergy( )
{
	m_money -= GetNextUpgradeCost( );
	uint16_t maxEnergy = CLASS_MAX_STATS[m_class][eStat_ENERGY];
	m_currentEnergy += MAX_ENERGY_INCREMENT;
	m_currentEnergy = Min( maxEnergy, m_currentEnergy );
}


//-------------------------------------------------------------------------------------------------
bool Player::HostAddItemToInventory( uint16_t itemCode )
{
	static bool const SUCCESS = true;
	static bool const FAIL = false;

	if( !g_GameSystem->IsHost( ) )
	{
		return FAIL;
	}

	for( byte_t inventoryIndex = 0; inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
	{
		if( m_inventory[inventoryIndex] == nullptr )
		{
			m_inventory[inventoryIndex] = new Item( itemCode );
			++m_inventorySize;
			return SUCCESS;
		}
	}
	return FAIL;
}


//-------------------------------------------------------------------------------------------------
void Player::HostRemoveItemFromInventory( byte_t itemIndex )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}
	
	if( m_inventory[itemIndex] )
	{
		delete m_inventory[itemIndex];
		m_inventory[itemIndex] = nullptr;
		--m_inventorySize;
	}
}


//-------------------------------------------------------------------------------------------------
void Player::HostEjectItemFromInventory( byte_t itemIndex )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	if( m_inventory[itemIndex] )
	{
		PlayerShip * playerShip = GetPlayerShip( );
		if( playerShip )
		{
			Vector2f backPosition = playerShip->GetPositionInfront( EJECT_DISTANCE );
			g_GameSystem->HostSpawnItem( backPosition, m_inventory[itemIndex]->m_itemCode );
			HostRemoveItemFromInventory( itemIndex );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Player::HostEquipItem( eEquipmentSlot const & slot, byte_t itemIndex )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	//Currently exists in my inventory
	if( m_inventory[itemIndex] )
	{
		uint16_t toBeEquippedItemCode = m_inventory[itemIndex]->m_itemCode;

		//If you already have something equipped
		if( m_equipment[slot] )
		{
			uint16_t equippedItemCode = m_equipment[slot]->m_itemCode;

			//Remove item from inventory
			HostRemoveItemFromInventory( itemIndex );

			//Equip item
			delete m_equipment[slot];
			m_equipment[slot] = new Item( toBeEquippedItemCode );

			//Add previously equipped item to inventory
			HostAddItemToInventory( equippedItemCode );
		}
		else
		{
			//Remove item from inventory
			HostRemoveItemFromInventory( itemIndex );

			//Equip item
			m_equipment[slot] = new Item( toBeEquippedItemCode );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Player::HostRemoveEquipment( eEquipmentSlot const & slot )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	if( m_equipment[slot] )
	{
		uint16_t equippedItemCode = m_equipment[slot]->m_itemCode;

		delete m_equipment[slot];
		m_equipment[slot] = nullptr;

		//Add previously equipped item to inventory
		HostAddItemToInventory( equippedItemCode );
	}
}


//-------------------------------------------------------------------------------------------------
void Player::HostResetPlayer( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	m_money = 0;
	m_lastLevel = 1;
	m_level = 0;
	m_class = ePlayerClass_NONE;
	m_currentHealth = 0;
	m_currentShield = 0;
	m_currentEnergy = 0;
	HostClearInventory( );
}


//-------------------------------------------------------------------------------------------------
void Player::HostClearInventory( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	for( byte_t inventoryIndex = 0; inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
	{
		if( m_inventory[inventoryIndex] )
		{
			delete m_inventory[inventoryIndex];
		}
		m_inventory[inventoryIndex] = nullptr;
	}
	m_inventorySize = 0;
}


//-------------------------------------------------------------------------------------------------
void Player::HostTurnIntoCthulhu( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	m_class = ePlayerClass_CTHULHU;
	PlayerShip * playerShip = GetPlayerShip( );
	if( playerShip )
	{
		playerShip->m_health = (uint16_t)playerShip->GetMaxHealth( );
		playerShip->m_shield = (uint16_t)playerShip->GetMaxShield( );
		playerShip->m_energy = (uint16_t)playerShip->GetMaxEnergy( );
	}
}


//-------------------------------------------------------------------------------------------------
void Player::ClientSetEquipment( eEquipmentSlot const & slot, uint16_t itemCode )
{
	//Equip an item
	if( itemCode != Item::INVALID_ITEM_CODE )
	{
		//Item already equipped
		if( m_equipment[slot] )
		{
			//It's not the item I want to equip
			if( m_equipment[slot]->m_itemCode != itemCode )
			{
				delete m_equipment[slot];
				m_equipment[slot] = new Item( itemCode );
				if( IsLocalClient( ) )
				{
					m_equipment[slot]->AttachToUIItem( m_equipmentContainer[slot] );
				}
			}
		}
		//No item currently equipped
		else
		{
			m_equipment[slot] = new Item( itemCode );
			if( IsLocalClient( ) )
			{
				m_equipment[slot]->AttachToUIItem( m_equipmentContainer[slot] );
			}
		}
	}

	//Unequip an item
	else if( itemCode == Item::INVALID_ITEM_CODE )
	{
		//If it's not already empty, remove it
		if( m_equipment[slot] != nullptr )
		{
			delete m_equipment[slot];
			m_equipment[slot] = nullptr;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Player::ClientAddOrChangeItemToInventory( byte_t inventoryIndex, uint16_t itemCode )
{
	//If item is not present
	if( !m_inventory[inventoryIndex] )
	{
		m_inventory[inventoryIndex] = new Item( itemCode );
		if( IsLocalClient( ) )
		{
			m_inventory[inventoryIndex]->AttachToUIItem( m_inventoryContainer );
		}
	}
	//If item in slot is wrong itemCode
	else if( m_inventory[inventoryIndex] )
	{
		if( m_inventory[inventoryIndex]->m_itemCode != itemCode )
		{
			//Remove it
			ClientRemoveItemFromInventory( inventoryIndex );
			//Add this one
			ClientAddOrChangeItemToInventory( inventoryIndex, itemCode );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Player::ClientRemoveItemFromInventory( byte_t inventoryIndex )
{
	//If item is present
	if( m_inventory[inventoryIndex] )
	{
		delete m_inventory[inventoryIndex];
	}
	m_inventory[inventoryIndex] = nullptr;
}


//-------------------------------------------------------------------------------------------------
void Player::CheckRespawn( )
{
	if( !HasShip( ) )
	{
		if( m_level > 0 )
		{
			g_GameSystem->HostSpawnPlayerShip( m_netIndex );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Player::SetInputState( uint16_t inputBitField )
{
	if( m_localShip )
	{
		PlayerShip * playerShip = (PlayerShip*) m_localShip->GetLocalObject( );
		playerShip->m_inputBitfieldState = inputBitField;
	}
}


//-------------------------------------------------------------------------------------------------
void Player::ApplyInputEvent( eGameEvent const & inputEvent )
{
	PlayerShip * playerShip = GetPlayerShip( );
	switch( inputEvent )
	{
	case eGameEvent_PRESS_SHOOT:
	{
		if( m_localShip )
		{
			if( playerShip->CanFire() )
			{
				playerShip->HostFireWeapon( );
				playerShip->ResetFireCooldown( );
			}
		}
		break;
	}
	case eGameEvent_UPGRADE_HULL:
		if( CanUpgradeHull( ) )
		{
			HostAddLevel( );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
		break;
	case eGameEvent_UPGRADE_HEALTH:
		if( CanUpgradeHealth( ) )
		{
			HostAddHealth( );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
		break;
	case eGameEvent_UPGRADE_SHIELD:
		if( CanUpgradeShield( ) )
		{
			HostAddShield( );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
		break;
	case eGameEvent_UPGRADE_ENERGY:
		if( CanUpgradeEnergy( ) )
		{
			HostAddEnergy( );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
		break;
	case eGameEvent_EMOTE_HAPPY:
		if( playerShip )
		{
			g_GameSystem->HostShowEmote( playerShip->m_position, playerShip->m_rotationDegrees, eEmoteType_HAPPY );
		}
		break;
	case eGameEvent_EMOTE_SAD:
		if( playerShip )
		{
			g_GameSystem->HostShowEmote( playerShip->m_position, playerShip->m_rotationDegrees, eEmoteType_SAD );
		}
		break;
	case eGameEvent_EMOTE_ANGRY:
		if( playerShip )
		{
			g_GameSystem->HostShowEmote( playerShip->m_position, playerShip->m_rotationDegrees, eEmoteType_ANGRY );
		}
		break;
	case eGameEvent_EMOTE_WHOOPS:
		if( playerShip )
		{
			g_GameSystem->HostShowEmote( playerShip->m_position, playerShip->m_rotationDegrees, eEmoteType_WHOOPS );
		}
		break;
	case eGameEvent_EMOTE_HELP:
		if( playerShip )
		{
			g_GameSystem->HostShowEmote( playerShip->m_position, playerShip->m_rotationDegrees, eEmoteType_HELP );
		}
		break;
	case eGameEvent_WARP_DRIVE:
	{
		if( playerShip )
		{
			playerShip->HostToggleWarping( );
		}
		break;
	}
	//#TODO: Set initial stats
	case eGameEvent_CHOOSE_CLASS_FIGHTER:
		if( CanChooseClass( ) )
		{
			HostSetStartingStats( ePlayerClass_FIGHTER );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
	case eGameEvent_CHOOSE_CLASS_WIZARD:
		if( CanChooseClass( ) )
		{
			HostSetStartingStats( ePlayerClass_WIZARD );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
	case eGameEvent_CHOOSE_CLASS_ROGUE:
		if( CanChooseClass( ) )
		{
			HostSetStartingStats( ePlayerClass_ROGUE );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
	case eGameEvent_CHOOSE_CLASS_CULTIST:
		if( CanChooseClass( ) )
		{
			HostSetStartingStats( ePlayerClass_CULTIST );
			g_GameSystem->HostUpdatePlayer( m_netIndex );
		}
	default:
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void Player::HostSetStartingStats( ePlayerClass const & playerClass )
{
	m_level = 1;
	m_class = playerClass;
	m_currentHealth = (uint16_t) ( (float) CLASS_MAX_STATS[playerClass][eStat_HEALTH] * BASE_STAT_STARTING_PERCENT );
	m_currentShield = (uint16_t) ( (float) CLASS_MAX_STATS[playerClass][eStat_SHIELD] * BASE_STAT_STARTING_PERCENT );
	m_currentEnergy = (uint16_t) ( (float) CLASS_MAX_STATS[playerClass][eStat_ENERGY] * BASE_STAT_STARTING_PERCENT );

	//Remove currently equipped weapon, and replace it with a new one
	if( m_equipment[eEquipmentSlot_PRIMARY] )
	{
		delete m_equipment[eEquipmentSlot_PRIMARY];
	}
	m_equipment[eEquipmentSlot_PRIMARY] = new Item( Item::GenerateRandomWeapon( 1U ) );
}


//-------------------------------------------------------------------------------------------------
uint8_t Player::CalculateDeltaBitfield( )
{
	if( m_forceUpdate )
	{
		m_forceUpdate = false;
		//Return that everything has changed
		return 0xFF;
	}

	uint8_t deltaBitfield = 0U;
	if( m_lastMoney != m_money )
	{
		SetBit( &deltaBitfield, 0 );
		m_lastMoney = m_money;
	}
	if( m_lastLevel != m_level )
	{
		SetBit( &deltaBitfield, 1 );
		m_lastLevel = m_level;
	}
	if( m_lastClass != m_class )
	{
		SetBit( &deltaBitfield, 2 );
		m_lastClass = m_class;
	}
	if( m_lastHealth != m_currentHealth )
	{
		SetBit( &deltaBitfield, 3 );
		m_lastHealth = m_currentHealth;
	}
	if( m_lastShield != m_currentShield )
	{
		SetBit( &deltaBitfield, 4 );
		m_lastShield = m_currentShield;
	}
	if( m_lastEnergy != m_currentEnergy )
	{
		SetBit( &deltaBitfield, 5 );
		m_lastEnergy = m_currentEnergy;
	}
	if( m_lastEnergy != m_currentEnergy )
	{
		SetBit( &deltaBitfield, 5 );
		m_lastEnergy = m_currentEnergy;
	}
	if( m_lastPrimaryEquipment != GetEquippedWeaponCode( ) )
	{
		SetBit( &deltaBitfield, 6 );
		m_lastPrimaryEquipment = GetEquippedWeaponCode( );
	}
	if( m_lastInventorySize != m_inventorySize )
	{
		SetBit( &deltaBitfield, 7 );
		m_lastInventorySize = m_inventorySize;
	}
	return deltaBitfield;
}


//-------------------------------------------------------------------------------------------------
uint16_t Player::CalculateInventoryBitfield( ) const
{
	uint16_t inventoryBitfield = 0U;
	for( uint8_t inventoryIndex = 0; inventoryIndex < MAX_INVENTORY_SIZE; ++inventoryIndex )
	{
		if( m_inventory[inventoryIndex] != nullptr )
		{
			SetBit( &inventoryBitfield, inventoryIndex );
		}
	}
	return inventoryBitfield;
}


//-------------------------------------------------------------------------------------------------
uint16_t Player::GetInputBitfield( ) const
{
	if( m_localShip )
	{
		return ( (PlayerShip*) ( m_localShip->GetLocalObject( ) ) )->m_inputBitfieldState;
	}
	else
	{
		return 0;
	}
}


//-------------------------------------------------------------------------------------------------
char const * Player::GetUsername( ) const
{
	return m_username;
}


//-------------------------------------------------------------------------------------------------
size_t Player::GetPassword( ) const
{
	return m_password;
}


//-------------------------------------------------------------------------------------------------
Color Player::GetColor( ) const
{
	return m_color;
}


//-------------------------------------------------------------------------------------------------
PlayerShip * Player::GetPlayerShip( ) const
{
	if( m_localShip )
	{
		return (PlayerShip*) m_localShip->GetLocalObject( );
	}
	else
	{
		return nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
byte_t Player::GetPlayerIndex( ) const
{
	return m_netIndex;
}


//-------------------------------------------------------------------------------------------------
uint16_t Player::GetNextUpgradeCost( ) const
{
	return m_level * CUBE_COST_PER_LEVEL;
}


//-------------------------------------------------------------------------------------------------
bool Player::GetInventoryIndexForItem( UIItem * findItem, byte_t * out_index ) const
{
	for( byte_t itemIndex = 0; itemIndex < MAX_INVENTORY_SIZE; ++itemIndex )
	{
		Item const * checkItem = m_inventory[itemIndex];
		if( checkItem != nullptr )
		{
			if( checkItem->m_boundUIItem == findItem )
			{
				*out_index = itemIndex;
				return true;
			}
		}
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
bool Player::GetEquipmentSlotForItem( UIItem * findItem, eEquipmentSlot * out_slot ) const
{
	for( byte_t equipmentIndex = 0; equipmentIndex < eEquipmentSlot_COUNT; ++equipmentIndex )
	{
		Item const * checkItem = m_equipment[equipmentIndex];
		if( checkItem && checkItem->m_boundUIItem == findItem )
		{
			*out_slot = (eEquipmentSlot)equipmentIndex;
			return true;
		}
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
byte_t Player::GetLastInventoryIndex( ) const
{
	for( byte_t inventoryIndex = MAX_INVENTORY_SIZE - 1; inventoryIndex >= 0; --inventoryIndex )
	{
		if( m_inventory[inventoryIndex] )
		{
			return inventoryIndex;
		}
	}
	return (byte_t) 0;
}


//-------------------------------------------------------------------------------------------------
uint16_t Player::GetEquippedWeaponCode( ) const
{
	if( m_equipment[eEquipmentSlot_PRIMARY] )
	{
		return m_equipment[eEquipmentSlot_PRIMARY]->m_itemCode;
	}
	
	return Item::INVALID_ITEM_CODE;
}


//-------------------------------------------------------------------------------------------------
Vector2f Player::GetLastPosition( ) const
{
	return m_loadPosition;
}


//-------------------------------------------------------------------------------------------------
bool Player::HasShip( ) const
{
	return m_localShip != nullptr;
}


//-------------------------------------------------------------------------------------------------
bool Player::CanUpgradeHull( ) const
{
	uint16_t amountNeededToLevel = GetNextUpgradeCost( );
	bool hasEnoughMoney = m_money > amountNeededToLevel;
	bool belowMaxLevel = m_level < MAX_LEVEL;
	return hasEnoughMoney && belowMaxLevel;
}


//-------------------------------------------------------------------------------------------------
bool Player::CanUpgradeHealth( ) const
{
	uint16_t amountNeededToUpgrade = GetNextUpgradeCost( );
	bool hasEnoughMoney = m_money > amountNeededToUpgrade;
	uint16_t maxHealth = CLASS_MAX_STATS[m_class][eStat_HEALTH];
	bool doesNotHaveMaxHealth = m_currentHealth < maxHealth;
	return hasEnoughMoney && doesNotHaveMaxHealth;
}


//-------------------------------------------------------------------------------------------------
bool Player::CanUpgradeShield( ) const
{
	uint16_t amountNeededToUpgrade = GetNextUpgradeCost( );
	bool hasEnoughMoney = m_money > amountNeededToUpgrade;
	uint16_t maxShield = CLASS_MAX_STATS[m_class][eStat_SHIELD];
	bool doesNotHaveMaxShield = m_currentShield < maxShield;
	return hasEnoughMoney && doesNotHaveMaxShield;
}


//-------------------------------------------------------------------------------------------------
bool Player::CanUpgradeEnergy( ) const
{
	uint16_t amountNeededToUpgrade = GetNextUpgradeCost( );
	bool hasEnoughMoney = m_money > amountNeededToUpgrade;
	uint16_t maxEnergy = CLASS_MAX_STATS[m_class][eStat_ENERGY];
	bool doesNotHaveMaxEnergy = m_currentEnergy < maxEnergy;
	return hasEnoughMoney && doesNotHaveMaxEnergy;
}


//-------------------------------------------------------------------------------------------------
bool Player::CanChooseClass( ) const
{
	return m_level == 0U;
}


//-------------------------------------------------------------------------------------------------
bool Player::ShouldSpawnFromLoad( ) const
{
	return m_spawnFromLoad;
}


//-------------------------------------------------------------------------------------------------
bool Player::IsMaxLevel( ) const
{
	return m_level == MAX_LEVEL;
}


//-------------------------------------------------------------------------------------------------
bool Player::IsHealthStatMaxed( ) const
{
	return m_currentHealth == CLASS_MAX_STATS[m_class][eStat_HEALTH];
}


//-------------------------------------------------------------------------------------------------
bool Player::IsShieldStatMaxed( ) const
{
	return m_currentShield == CLASS_MAX_STATS[m_class][eStat_SHIELD];
}


//-------------------------------------------------------------------------------------------------
bool Player::IsEnergyStatMaxed( ) const
{
	return m_currentEnergy == CLASS_MAX_STATS[m_class][eStat_ENERGY];
}


//-------------------------------------------------------------------------------------------------
bool Player::IsLocalClient( ) const
{
	return g_GameSystem->ClientGetPlayerSelf( ) == this;
}


//-------------------------------------------------------------------------------------------------
bool Player::IsEvilClass( ) const
{
	return ( m_class == ePlayerClass_CULTIST ) || ( m_class == ePlayerClass_CTHULHU );
}


//-------------------------------------------------------------------------------------------------
void Player::LoadLastData( PlayerShip * ship )
{
	ship->m_position = m_loadPosition;
	ship->m_rotationDegrees = m_loadRotation;
	ship->m_health = m_loadHealth;
	m_spawnFromLoad = false;
}


//-------------------------------------------------------------------------------------------------
void Player::SetLastPosition( )
{
	PlayerShip * myShip = GetPlayerShip( );
	if( myShip )
	{
		m_loadPosition = myShip->m_position;
	}
}
