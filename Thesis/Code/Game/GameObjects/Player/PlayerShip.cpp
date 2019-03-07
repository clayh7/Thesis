#include "Game/GameObjects/Player/PlayerShip.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleSystem.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Game/General/Game.hpp"
#include "Game/GameObjects/Enemies/Rock.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/Items/Pickup.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Other/Emote.hpp"
#include "Game/GameObjects/Player/Player.hpp"
#include "Game/General/GameCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * PlayerShip::NAME = "PlayerShip";
STATIC float const PlayerShip::NET_SYNC_PERCENT = 0.15f;
STATIC float const PlayerShip::SPEED = 3.f;
STATIC float const PlayerShip::SPEED_DECAY_RATE = 0.05f;
STATIC float const PlayerShip::TURN_SPEED = 90.0f;
STATIC float const PlayerShip::RADIUS = 0.5f;
STATIC float const PlayerShip::PLAYERSHIP_RADIUS_TO_SCALE = 3.f;
STATIC float const PlayerShip::FIRE_RATE = 0.5f;
STATIC float const PlayerShip::WARP_TIME_BASE = 3.f;
STATIC float const PlayerShip::WARP_TIME_PER_ITEM = 1.5f;
STATIC float const PlayerShip::TARGET_RADIUS_DEGREES = 20.f;
STATIC float const PlayerShip::TARGET_MAX_RANGE = 12.f;
STATIC byte_t const PlayerShip::TARGET_KEY = 'Q';
STATIC char const * PlayerShip::ID_LOCKED_ON = "lockedOn";
STATIC char const * PlayerShip::ID_TARGETING = "targeting";
STATIC float const PlayerShip::SHIELD_REGEN_COOLDOWN = 0.2f;
STATIC float const PlayerShip::ENERGY_REGEN_COOLDOWN = 0.2f;
STATIC float const PlayerShip::ROGUE_SPEED_MULTIPLIER = 1.35f;
STATIC int const PlayerShip::CTHULHU_HEALTH = 2000;
STATIC int const PlayerShip::CTHULHU_SHIELD = 1000;
STATIC int const PlayerShip::CTHULHU_ENERGY = 1000;
STATIC float const PlayerShip::CTHULHU_RADIUS = 2.0f;
STATIC float const PlayerShip::CTHULHU_SPEED = 5.f;
STATIC float const PlayerShip::CTHULHU_FIRE_RATE = 0.25f;
STATIC float const PlayerShip::CRYSTAL_FEED_RATE = 0.5f;
STATIC float const PlayerShip::CRYSTAL_FEED_RANGE = 4.0f;
STATIC uint16_t const PlayerShip::CRYSTAL_FEED_AMOUNT = 10U;


STATIC GameObjectRegistration PlayerShip::s_PlayerShipMessageRegistration( eNetGameObjectType_PLAYERSHIP, &PlayerShip::MessageCreation );


//-------------------------------------------------------------------------------------------------
STATIC GameObject * PlayerShip::MessageCreation( NetSender const & sender, NetMessage const & message )
{
	return new PlayerShip( sender, message );
}


//-------------------------------------------------------------------------------------------------
PlayerShip::PlayerShip( NetSender const & sender, NetMessage const & message )
	: PlayerShip( INVALID_INDEX, false )
{
	UpdateFromMessage( sender, message );
	m_inputBitfieldState = m_netInputBitfieldState;
	m_position = m_netPosition;
	m_velocity = m_netVelocity;
	m_rotationDegrees = m_netRotationDegrees;
	m_health = m_netHealth;
	m_shield = m_netShield;
	m_energy = m_netEnergy;
	m_isWarping = m_netIsWarping;
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::WriteToMessage( NetMessage * out_message )
{
	Ship::WriteToMessage( out_message );

	Game::WriteUncompressedUint8( out_message, m_playerIndex );
	Game::WriteUncompressedUint16( out_message, m_inputBitfieldState );
	Game::WriteCompressedBoolean( out_message, m_isWarping );
}


//-------------------------------------------------------------------------------------------------
//Client Only
void PlayerShip::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Ship::UpdateFromMessage( sender, message );

	Game::ReadUncompressedUint8( message, &m_playerIndex );
	Game::ReadUncompressedUint16( message, &m_netInputBitfieldState );
	Game::ReadCompressedBoolean( message, &m_netIsWarping );
}


//-------------------------------------------------------------------------------------------------
PlayerShip::PlayerShip( byte_t playerIndex, bool hostObject )
	: Ship( playerIndex, eNetGameObjectType_PLAYERSHIP, hostObject )
	, m_inputBitfieldState( 0U )
	, m_netInputBitfieldState( 0U )
	, m_isWarping( false )
	, m_netIsWarping( false )
	, m_warpTimer( 0.0f )
	, m_shieldRegenTimer( 0.f )
	, m_energyRegenTimer( 0.f )
	, m_lockedOn( false )
	, m_targetGameObject( nullptr )
{
	m_health = (uint16_t) GetMaxHealth( );
	m_netHealth = (uint16_t) GetMaxHealth( );
	m_shield = (uint16_t) GetMaxShield( );
	m_netShield = (uint16_t) GetMaxShield( );
	m_energy = (uint16_t) GetMaxEnergy( );
	m_netEnergy = (uint16_t) GetMaxEnergy( );

	m_weaponSprite = SpriteGameRenderer::Create( "nothing", LAYER_WEAPON );
}


//-------------------------------------------------------------------------------------------------
PlayerShip::~PlayerShip( )
{
	if( m_weaponSprite != nullptr )
	{
		delete m_weaponSprite;
		m_weaponSprite = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::Update( )
{
	UpdateTimers( );

	//Host Update
	if( m_isHostObject )
	{
		if( m_warpTimer > GetMaxWarpTime( ) )
		{
			//Maybe turn this into Warp();
			//#TODO: Update to be saved warp location
			Vector2f warpLocation = Vector2f::ZERO;
			m_position = warpLocation;
			//Maybe activate Immunity

			HostToggleWarping( );
		}

		//Regen Shield
		if( m_shieldRegenTimer > SHIELD_REGEN_COOLDOWN )
		{
			m_shieldRegenTimer -= SHIELD_REGEN_COOLDOWN;

			if( m_shield < GetMaxShield( ) )
			{
				m_shield += 1;
			}
		}

		//Regen Energy
		if( m_energyRegenTimer > ENERGY_REGEN_COOLDOWN )
		{
			m_energyRegenTimer -= ENERGY_REGEN_COOLDOWN;

			if( m_energy < GetMaxEnergy( ) )
			{
				m_energy += 1;
			}
		}
	}

	//Client Update
	else if( !m_isHostObject )
	{
		//Update synced members
		bool previousWarpingStatus = IsWarping( );
		m_isWarping = m_netIsWarping;
		bool newWarpingStatus = IsWarping( );
		if( previousWarpingStatus != newWarpingStatus )
		{
			m_warpTimer = 0.f;
		}

		//Update current target
		if( !m_lockedOn )
		{
			g_GameSystem->ClientGetClosestGameObjectWithinDegrees( this, TARGET_RADIUS_DEGREES, &m_targetGameObject );
		}

		//Disconnect target if range is too far
		if( m_targetGameObject )
		{
			Vector2f vectorToTarget = m_targetGameObject->m_position - m_position;
			if( vectorToTarget.Length( ) > TARGET_MAX_RANGE )
			{
				m_targetGameObject = nullptr;
				m_lockedOn = false;
			}
		}

		//Toggle Lock-on
		if( g_InputSystem->WasKeyJustPressed( TARGET_KEY ) )
		{
			if( m_targetGameObject )
			{
				m_lockedOn = !m_lockedOn;
			}
		}
	}

	//Move Ship
	Ship::Update( );
	//Post-Move Update

	//Host Update
	if( m_isHostObject )
	{
		float distanceFromCenter = m_position.Length( );

		// Player leave map
		if( distanceFromCenter > Game::MAX_MAP_RADIUS )
		{
			//#TODO: Make this less severe?
			//Kill player
			m_health = 0;
		}

// 		Player feed crystal
// 		if( CanFeedCrystal( ) )
// 		{
// 			g_GameSystem->HostPlayerFeedCrystal( m_playerIndex, CRYSTAL_FEED_AMOUNT );
// 			m_feedCrystalTimer = CRYSTAL_FEED_RATE;
// 		}
	}

	//Client Update
	else if( !m_isHostObject )
	{
		UpdateWeaponSprite( );
	}
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::HostHandleCollision( GameObject * gameObject )
{
	//I would do this, but this is being called 1000x1000 times a frame
	//and I don't want to cause it to go any slower and I know this will always be true
	//if( !g_GameSystem->IsHost( ) )
	//{
	//	return;
	//}

	//#TODO: Could make improvements to collision check in game,
	//giving every objects position a bit field and comparing the
	//possible areas where those objects could be beforehand

	// Collision with Pickups
	if( gameObject->m_type == eNetGameObjectType_PICKUP )
	{
		// Check collision
		Pickup * hitPickup = (Pickup*) gameObject;
		//If it's already picked up, ignore it
		if( hitPickup->m_taken )
		{
			return;
		}

		Player * owningPlayer = g_GameSystem->HostGetPlayerForPlayerShip( this );

		//If item is added to inventory
		if( g_GameSystem->HostAddItemToPlayer( owningPlayer, hitPickup->m_itemCode ) )
		{
			hitPickup->m_taken = true;
		}
	}

	// Check parent's collision
	else
	{
		Ship::HostHandleCollision( gameObject );
	}
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::HostFireWeapon( ) const
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	Player * owner = g_GameSystem->HostGetPlayer( m_playerIndex );
	if( owner )
	{
		uint16_t weaponCode = owner->GetEquippedWeaponCode( );
		float radius = GetPhysicsRadius( );
		//Move to the nose of the ship
		Vector2f bulletPosition = GetPositionInfront( BULLET_START_OFFSET * radius );
		//Special case for bombers
		if( Item::ParseItemType( weaponCode ) == eItemType_WEAPON_BOMBER )
		{
			bulletPosition = GetPositionInfront( -BULLET_START_OFFSET * radius );
		}
		g_GameSystem->HostFireWeapon( m_playerIndex, weaponCode, bulletPosition, m_rotationDegrees, m_velocity );
	}
}


//-------------------------------------------------------------------------------------------------
char const * PlayerShip::GetEntityName( ) const
{
	return NAME;
}


//-------------------------------------------------------------------------------------------------
std::string PlayerShip::GetSpriteID( ) const
{
	ePlayerClass playerClass = GetPlayerClass( );
	return Player::GetSpriteIDForClass( playerClass );
}


//-------------------------------------------------------------------------------------------------
float PlayerShip::GetNetSyncPercent( ) const
{
	return NET_SYNC_PERCENT;
}


//-------------------------------------------------------------------------------------------------
float PlayerShip::GetPhysicsRadius( ) const
{
	//Special case of cthulhu
	ePlayerClass playerClass = GetPlayerClass( );
	if( playerClass == ePlayerClass_CTHULHU )
	{
		return CTHULHU_RADIUS;
	}

	return RADIUS;
}


//-------------------------------------------------------------------------------------------------
float PlayerShip::GetSpriteScale( ) const
{
	//Special case of cthulhu
	ePlayerClass playerClass = GetPlayerClass( );
	if( playerClass == ePlayerClass_CTHULHU )
	{
		return ConvertToSpriteScale( CTHULHU_RADIUS, PLAYERSHIP_RADIUS_TO_SCALE );
	}

	return ConvertToSpriteScale( RADIUS, PLAYERSHIP_RADIUS_TO_SCALE );
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::HostOnDestroy( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	Ship::HostOnDestroy( );

	ePlayerClass playerClass = GetPlayerClass( );
	if( playerClass == ePlayerClass_CTHULHU )
	{
		g_GameSystem->HostResetCthulhuSummoning( );
	}

	Player * player = g_GameSystem->HostGetPlayer( m_playerIndex );
	if( player )
	{
		//Drop currently equip weapon
		if( !HostIsAlive( ) )
		{
			uint16_t weaponCode = player->GetEquippedWeaponCode( );
			if( weaponCode != Item::INVALID_ITEM_CODE )
			{
				g_GameSystem->HostSpawnItem( m_position, weaponCode );
			}
		}

		player->m_localShip = nullptr;
		player->HostResetPlayer( );

		//Tell everyone that you you have nothing
		g_GameSystem->HostUpdatePlayer( m_playerIndex );
	}

}


//-------------------------------------------------------------------------------------------------
void PlayerShip::ClientOnDestroy( )
{
	Player * clientPlayer = g_GameSystem->ClientGetPlayer( m_playerIndex );
	if( clientPlayer )
	{
		//Clear player ship ref because it's about to be deleted and then this would be pointing at "valid" bad data
		clientPlayer->m_localShip = nullptr;
	}

	Ship::ClientOnDestroy( );
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::UpdateTimers( )
{
	//Warp Spin Up
	if( m_isWarping )
	{
		m_warpTimer += Time::DELTA_SECONDS;
	}
	m_shieldRegenTimer += Time::DELTA_SECONDS;
	m_energyRegenTimer += Time::DELTA_SECONDS;
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::UpdateWeaponSprite( )
{
	Player * owner = g_GameSystem->ClientGetPlayer( m_playerIndex );
	if( owner )
	{
		uint16_t weaponItemCode = owner->GetEquippedWeaponCode( );
		if( weaponItemCode == Item::INVALID_ITEM_CODE )
		{
			m_weaponSprite->SetID( "nothing" );
			return;
		}
		m_weaponSprite->SetID( Item::ParseSprite( weaponItemCode ) );
		m_weaponSprite->SetScale( GetSpriteScale( ) );
		m_weaponSprite->SetPosition( m_position );
		m_weaponSprite->SetRotation( -m_rotationDegrees );
	}
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::ApplyControlInput( )
{
	//Host Input
	if( m_isHostObject )
	{
		//Host moves host version of clients ships as if they own it
		//Inputs received from clients are stored in m_inputBitfieldState
		ApplyInput( m_inputBitfieldState );
	}

	//Client Input
	else if( !m_isHostObject )
	{
		//Moves the ship normally
		if( IsClientOwner( ) )
		{
			ApplyInput( m_inputBitfieldState );
		}
		//Move based on net input
		else
		{
			ApplyInput( m_netInputBitfieldState );
		}
	}
}


//-------------------------------------------------------------------------------------------------
int PlayerShip::GetMaxHealth( ) const
{
	//Get Owner
	Player * shipOwner = nullptr;
	if( m_isHostObject )
	{
		shipOwner = g_GameSystem->HostGetPlayer( m_playerIndex );
	}
	else
	{
		shipOwner = g_GameSystem->ClientGetPlayer( m_playerIndex );
	}

	ePlayerClass playerClass = ePlayerClass_NONE;
	if( shipOwner )
	{
		playerClass = shipOwner->m_class;
	}

	//Doesn't exist yet
	if( playerClass == ePlayerClass_NONE )
	{
		return 1;
	}
	
	//Special case of cthulhu
	if( playerClass == ePlayerClass_CTHULHU )
	{
		return CTHULHU_HEALTH;
	}

	//Get health from owner
	return shipOwner->m_currentHealth;
}


//-------------------------------------------------------------------------------------------------
int PlayerShip::GetMaxShield( ) const
{
	//Get Owner
	Player * shipOwner = nullptr;
	if( m_isHostObject )
	{
		shipOwner = g_GameSystem->HostGetPlayer( m_playerIndex );
	}
	else
	{
		shipOwner = g_GameSystem->ClientGetPlayer( m_playerIndex );
	}

	ePlayerClass playerClass = ePlayerClass_NONE;
	if( shipOwner )
	{
		playerClass = shipOwner->m_class;
	}

	//Doesn't exist yet
	if( playerClass == ePlayerClass_NONE )
	{
		return 1;
	}

	//Special case of cthulhu
	if( playerClass == ePlayerClass_CTHULHU )
	{
		return CTHULHU_SHIELD;
	}

	//Get shield from owner
	return shipOwner->m_currentShield;
}


//-------------------------------------------------------------------------------------------------
int PlayerShip::GetMaxEnergy( ) const
{
	//Get Owner
	Player * shipOwner = nullptr;
	if( m_isHostObject )
	{
		shipOwner = g_GameSystem->HostGetPlayer( m_playerIndex );
	}
	else
	{
		shipOwner = g_GameSystem->ClientGetPlayer( m_playerIndex );
	}

	ePlayerClass playerClass = ePlayerClass_NONE;
	if( shipOwner )
	{
		playerClass = shipOwner->m_class;
	}

	//Doesn't exist yet
	if( playerClass == ePlayerClass_NONE )
	{
		return 1;
	}

	//Special case of cthulhu
	if( playerClass == ePlayerClass_CTHULHU )
	{
		return CTHULHU_ENERGY;
	}

	//Get energy from owner
	return shipOwner->m_currentEnergy;
}


//-------------------------------------------------------------------------------------------------
float PlayerShip::GetMaxSpeed( ) const
{
	//Get Owner
	Player * shipOwner = nullptr;
	if( m_isHostObject )
	{
		shipOwner = g_GameSystem->HostGetPlayer( m_playerIndex );
	}
	else
	{
		shipOwner = g_GameSystem->ClientGetPlayer( m_playerIndex );
	}

	ePlayerClass playerClass = ePlayerClass_NONE;
	if( shipOwner )
	{
		playerClass = shipOwner->m_class;
	}

	//Special case of cthulhu
	if( playerClass == ePlayerClass_CTHULHU )
	{
		return CTHULHU_SPEED;
	}

	float maxSpeed = SPEED;
	if( playerClass == ePlayerClass_ROGUE )
	{
		maxSpeed *= ROGUE_SPEED_MULTIPLIER;
	}

	//Speed based on how slow your are
	if( g_GameSystem->IsHost( ) )
	{
		if( HostIsSlowed( ) )
		{
			return maxSpeed * m_slowAmount;
		}
	}
	return maxSpeed;
}


//-------------------------------------------------------------------------------------------------
float PlayerShip::GetFireRate( ) const
{
	//Get Owner
	Player * shipOwner = nullptr;
	if( m_isHostObject )
	{
		shipOwner = g_GameSystem->HostGetPlayer( m_playerIndex );
	}
	else
	{
		shipOwner = g_GameSystem->ClientGetPlayer( m_playerIndex );
	}

	ePlayerClass playerClass = ePlayerClass_NONE;
	if( shipOwner )
	{
		playerClass = shipOwner->m_class;
	}

	//Special case of cthulhu
	if( playerClass == ePlayerClass_CTHULHU )
	{
		return CTHULHU_FIRE_RATE;
	}
	
	if( shipOwner )
	{
		Item * primaryWeapon = shipOwner->m_equipment[eEquipmentSlot_PRIMARY];
		if( primaryWeapon )
		{
			uint16_t itemCode = primaryWeapon->m_itemCode;
			if( itemCode != Item::INVALID_ITEM_CODE )
			{
				return Item::ParseFireRate( itemCode );
			}
		}
	}
	return FIRE_RATE;
}


//-------------------------------------------------------------------------------------------------
uint8_t PlayerShip::GetSummoningBarValue( ) const
{
	ePlayerClass playerClass = GetPlayerClass( );
	if( playerClass != ePlayerClass_CULTIST )
	{
		return 10U;
	}
	return 0U;
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::ApplyInput( uint16_t inputBitfield )
{
	//Apply turning
	if( IsBitSet( inputBitfield, eGameButton_LEFT ) )
	{
		m_rotationDegrees += TURN_SPEED * Time::DELTA_SECONDS;
	}
	if( IsBitSet( inputBitfield, eGameButton_RIGHT ) )
	{
		m_rotationDegrees -= TURN_SPEED * Time::DELTA_SECONDS;
	}

	//Apply movement based on input state
	if( IsBitSet( inputBitfield, eGameButton_UP ) )
	{
		m_velocity += UnitVectorFromDegrees( m_rotationDegrees ) * ( GetMaxSpeed( ) * Time::DELTA_SECONDS );
	}
	if( IsBitSet( inputBitfield, eGameButton_DOWN ) )
	{
		m_velocity -= UnitVectorFromDegrees( m_rotationDegrees ) * ( GetMaxSpeed( ) * Time::DELTA_SECONDS );
	}

	//Press nothing
	if( !IsBitSet( inputBitfield, eGameButton_UP ) &&
		!IsBitSet( inputBitfield, eGameButton_DOWN ) )
	{
		m_velocity *= ( 1 - SPEED_DECAY_RATE );
	}

	m_velocity = m_velocity.Normal() * Clamp( m_velocity.Length(), 0.f, GetMaxSpeed( ) );
}


//-------------------------------------------------------------------------------------------------
void PlayerShip::HostToggleWarping( )
{
	m_isWarping = !m_isWarping;
	m_warpTimer = 0.f;
	Player * owningPlayer = g_GameSystem->HostGetPlayer( m_playerIndex );
	if( owningPlayer )
	{
		g_GameSystem->m_HostGameActivityLog.Printf( "%s (%u) %s", owningPlayer->GetUsername( ), owningPlayer->GetPlayerIndex( ), m_isWarping ? "Started Warping" : "Stopped Warping" );
	}
}


//-------------------------------------------------------------------------------------------------
bool PlayerShip::IsWarping( ) const
{
	return m_isWarping;
}


//-------------------------------------------------------------------------------------------------
bool PlayerShip::CanFeedCrystal( ) const
{
	bool isGood = !HostIsEvil( );
	float distanceFromCenter = m_position.Length( );
	bool closeEnough = distanceFromCenter < CRYSTAL_FEED_RANGE;
	Player const * owningPlayer = g_GameSystem->HostGetPlayerForPlayerShip( this );
	bool enoughMoney = owningPlayer->m_money >= CRYSTAL_FEED_AMOUNT;
	bool isNotEnemy = !owningPlayer->IsEvilClass( );
	return isGood && closeEnough && enoughMoney && isNotEnemy;
}


//-------------------------------------------------------------------------------------------------
float PlayerShip::GetCurrentWarpTime( ) const
{
	return m_warpTimer;
}


//-------------------------------------------------------------------------------------------------
float PlayerShip::GetMaxWarpTime( ) const
{
	Player * shipOwner = g_GameSystem->ClientGetPlayer( m_playerIndex );
	if( shipOwner )
	{
		int itemCount = (int) shipOwner->m_inventorySize;
		return WARP_TIME_BASE + WARP_TIME_PER_ITEM * itemCount;
	}
	else
	{
		return WARP_TIME_BASE;
	}
}


//-------------------------------------------------------------------------------------------------
ePlayerClass PlayerShip::GetPlayerClass( ) const
{
	Player * checkPlayer = g_GameSystem->ClientGetPlayer( m_playerIndex );
	ePlayerClass playerClass = ePlayerClass_NONE;
	if( checkPlayer )
	{
		playerClass = checkPlayer->m_class;
	}
	return playerClass;
}


//-------------------------------------------------------------------------------------------------
bool PlayerShip::HasTarget( ) const
{
	return m_targetGameObject != nullptr;
}


//-------------------------------------------------------------------------------------------------
GameObject * PlayerShip::GetTarget( ) const
{
	return m_targetGameObject;
}


//-------------------------------------------------------------------------------------------------
std::string PlayerShip::GetTargetBracketSpriteID( ) const
{
	if( m_lockedOn )
	{
		return ID_LOCKED_ON;
	}
	else
	{
		return ID_TARGETING;
	}
}


//-------------------------------------------------------------------------------------------------
bool PlayerShip::HostIsEvil( ) const
{
	if( !g_GameSystem->IsHost( ) )
	{
		return false;
	}

	Player * checkPlayer = g_GameSystem->HostGetPlayer( m_playerIndex );
	if( checkPlayer )
	{
		return checkPlayer->IsEvilClass( );
	}
	return false;
}