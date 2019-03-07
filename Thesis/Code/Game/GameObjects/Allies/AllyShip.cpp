#include "Game/GameObjects/Allies/AllyShip.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/General/GameState.hpp"
#include "Game/General/Game.hpp"
#include "Game/GameObjects/Enemies/ShipSpawner.hpp"
#include "Game/GameObjects/Enemies/EnemyShip.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * AllyShip::NAME = "AllyShip";
STATIC char const * AllyShip::CRYSTAL_NAME = "Crystal";
STATIC float const AllyShip::NET_SYNC_PERCENT = 1.0f;
STATIC float const AllyShip::RADIUS = 0.5f;
STATIC float const AllyShip::SIGHT_RADIUS = 10.f;
STATIC float const AllyShip::SAFE_DISTANCE = 4.0f;
STATIC float const AllyShip::SAFE_RADIUS = 0.5f; //(safe zone Max - safe zone Min)
STATIC float const AllyShip::SPEED = 1.0f;
STATIC float const AllyShip::SPEED_DECAY_RATE = 0.05f;
STATIC float const AllyShip::TURN_SPEED = 45.f;
STATIC float const AllyShip::FIRE_RATE_SECONDS = 1.0f;
STATIC float const AllyShip::MAX_WANDER_DISTANCE = 5.0f;
STATIC float const AllyShip::WANDER_THRESHOLD = 1.f;
STATIC float const AllyShip::SHIELD_REGEN_COOLDOWN = 1.0f;
STATIC uint16_t const AllyShip::MAX_HEALTH = 200U;
STATIC uint16_t const AllyShip::MAX_SHIELD = 100U;
STATIC uint16_t const AllyShip::MAX_ENERGY = 50U;
STATIC float const AllyShip::CRYSTAL_RADIUS = 2.0f;
STATIC uint16_t const AllyShip::CRYSTAL_MAX_HEALTH = 5000U;
STATIC uint16_t const AllyShip::CRYSTAL_MAX_SHIELD = 0U;
STATIC uint16_t const AllyShip::CRYSTAL_MAX_ENERGY = 0U;
STATIC GameObjectRegistration AllyShip::s_AllyXMLRegistration( NAME, &AllyShip::XMLCreation );
STATIC GameObjectRegistration AllyShip::s_AllyMessageRegistration( eNetGameObjectType_ALLYSHIP, &AllyShip::MessageCreation );


//-------------------------------------------------------------------------------------------------
STATIC GameObject * AllyShip::MessageCreation( NetSender const & sender, NetMessage const & message )
{
	return new AllyShip( sender, message );
}


//-------------------------------------------------------------------------------------------------
STATIC GameObject * AllyShip::XMLCreation( XMLNode const & node )
{
	return new AllyShip( node );
}


//-------------------------------------------------------------------------------------------------
AllyShip::AllyShip( NetSender const & sender, NetMessage const & message )
	: AllyShip( eAllyType_INVALID, false )
{
	UpdateFromMessage( sender, message );
	m_position = m_netPosition;
	m_velocity = m_netVelocity;
	m_rotationDegrees = m_netRotationDegrees;
	m_health = m_netHealth;

	m_wanderTarget = m_position;

	if( IsCrystal( ) )
	{
		GameState::SetCrystal( this, m_isHostObject );
		m_sprite->SetLayer( LAYER_CRYSTAL );
	}
}


//-------------------------------------------------------------------------------------------------
//Host Only
void AllyShip::WriteToMessage( NetMessage * out_message )
{
	Ship::WriteToMessage( out_message );

	Game::WriteUncompressedUint8( out_message, m_allyType );
}


//-------------------------------------------------------------------------------------------------
//Client Only
void AllyShip::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Ship::UpdateFromMessage( sender, message );

	Game::ReadUncompressedUint8( message, (uint8_t*) &m_allyType );

	if( IsCrystal( ) )
	{
		CreateOrUpdateCrystalBubble( );
	}

	//Move things to where they probably are on the host at this moment in time
	UpdateNetData( GetElapsedGameTime( sender ) );
	MessageRecieved( );
}


//-------------------------------------------------------------------------------------------------
AllyShip::AllyShip( XMLNode const & node )
	: AllyShip( eAllyType_INVALID, true )
{
	UpdateFromXMLNode( node );

	m_wanderTarget = m_position;

	if( IsCrystal( ) )
	{
		GameState::SetCrystal( this, m_isHostObject );
	}
}


//-------------------------------------------------------------------------------------------------
//Host Only
void AllyShip::WriteToXMLNode( XMLNode * out_xmlNode )
{
	XMLNode node = out_xmlNode->addChild( NAME );
	node.addAttribute( STRING_ALLY_TYPE, Stringf( "%u", m_allyType ).c_str( ) );

	if( !IsCrystal( ) )
	{
		Ship::WriteToXMLNode( &node );
	}
	else if( IsCrystal( ) )
	{
		CreateOrUpdateCrystalBubble( );
		node.addAttribute( STRING_HEALTH, Stringf( "%u", m_health ).c_str( ) );
		GameObject::WriteToXMLNode( &node );
	}
}


//-------------------------------------------------------------------------------------------------
//Host Only
void AllyShip::UpdateFromXMLNode( XMLNode const & node )
{
	m_allyType = (eAllyType) ReadXMLAttribute( node, STRING_ALLY_TYPE, (byte_t) eAllyType_INVALID );

	Ship::UpdateFromXMLNode( node );
}


//-------------------------------------------------------------------------------------------------
AllyShip::AllyShip( eAllyType const & allyType, bool hostObject )
	: Ship( ALLY_PLAYER_INDEX, eNetGameObjectType_ALLYSHIP, hostObject )
	, m_allyType( allyType )
	, m_crystalBubble( nullptr )
	, m_aiTimer( 0.f )
	, m_shieldRegenTimer( 0.f )
	, m_targetShip( nullptr )
	, m_wanderTarget( m_position )
{
	if( m_allyType != eAllyType_INVALID )
	{
		m_health = (uint16_t) GetMaxHealth( );
		m_netHealth = (uint16_t) GetMaxHealth( );
		m_shield = (uint16_t) GetMaxShield( );
		m_netShield = (uint16_t) GetMaxShield( );
		m_energy = (uint16_t) GetMaxEnergy( );
		m_netEnergy = (uint16_t) GetMaxEnergy( );

		if( IsCrystal( ) )
		{
			CreateOrUpdateCrystalBubble( );
			GameState::SetCrystal( this, m_isHostObject );
			m_sprite->SetLayer( LAYER_CRYSTAL );
		}
	}
}


//-------------------------------------------------------------------------------------------------
AllyShip::~AllyShip( )
{
	ShipSpawner::RemoveFromSpawner( this );

	delete m_crystalBubble;
	m_crystalBubble = nullptr;
}


//-------------------------------------------------------------------------------------------------
void AllyShip::Update( )
{
	UpdateTimers( );

	//Host Object
	if( m_isHostObject )
	{
		//Regen Shield
		if( m_shieldRegenTimer > SHIELD_REGEN_COOLDOWN )
		{
			m_shieldRegenTimer -= SHIELD_REGEN_COOLDOWN;

			if( m_shield < GetMaxShield( ) )
			{
				m_shield += 1;
			}
		}
	}

	//Client Update
	else if( !m_isHostObject )
	{
		if( IsCrystal( ) )
		{
			m_netPosition = 0.f;
			m_netRotationDegrees = 0.f;
			m_position = 0.f;
			m_velocity = 0.f;
			m_rotationDegrees = 0.f;
		}
	}

	//Move Ship
	Ship::Update( );
	//Post-Move Update

	//Host Update
	if( m_isHostObject )
	{
		//Leave map
		if( m_position.Length( ) > Game::MAX_MAP_RADIUS )
		{
			//Kill Enemy
			m_health = 0;
		}
	}

	if( IsCrystal( ) )
	{
		CreateOrUpdateCrystalBubble( );
	}
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostHandleCollision( GameObject * gameObject )
{
	// Don't collide with friendly objects
	if( gameObject->m_playerIndex == m_playerIndex )
	{
		return;
	}

	// Crystal collisions are handled on the bullet
	else if( IsCrystal( ) )
	{
		return;
	}

	// Check parent's collision
	else
	{
		Ship::HostHandleCollision( gameObject );
	}
}


//-------------------------------------------------------------------------------------------------
bool AllyShip::HostIsAlive( ) const
{
	return m_health > 0;
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostFireWeapon( ) const
{
	//Get itemCode for allyship
	uint16_t weaponCode = Item::GetItemCode( eItemType_WEAPON_PHASER, 10, 8 ); //specifically 8 because the damage is the highest
	Bullet * firedBullet = new Bullet( m_playerIndex, weaponCode, m_isHostObject );
	//Move to the nose of the ship
	float radius = GetPhysicsRadius( );
	Vector2f bulletPosition = GetPositionInfront( BULLET_START_OFFSET * radius );
	firedBullet->m_position = bulletPosition;
	firedBullet->m_rotationDegrees = m_rotationDegrees;
	firedBullet->m_velocity = UnitVectorFromDegrees( m_rotationDegrees ) * Bullet::SPEED;
	g_GameSystem->HostCreateNetGameObject( firedBullet );
}


//-------------------------------------------------------------------------------------------------
char const * AllyShip::GetEntityName( ) const
{
	if( IsCrystal( ) )
	{
		return "Crystal (Good)";
	}
	else
	{
		return "Sentinel (Good)";
	}
}


//-------------------------------------------------------------------------------------------------
std::string AllyShip::GetSpriteID( ) const
{
	switch( m_allyType )
	{
	case eAllyType_CRYSTAL:
		return "crystal";
	case eAllyType_GUARD:
		return "allyGuard";
	default:
		return "error";
	}
}


//-------------------------------------------------------------------------------------------------
float AllyShip::GetNetSyncPercent( ) const
{
	return NET_SYNC_PERCENT;
}


//-------------------------------------------------------------------------------------------------
float AllyShip::GetPhysicsRadius( ) const
{
	switch( m_allyType )
	{
	case eAllyType_CRYSTAL:
		return CRYSTAL_RADIUS;
	default:
		return RADIUS;
	}
}


//-------------------------------------------------------------------------------------------------
float AllyShip::GetSpriteScale( ) const
{
	switch( m_allyType )
	{
	case eAllyType_CRYSTAL:
		return ConvertToSpriteScale( CRYSTAL_RADIUS, RADIUS_TO_SCALE );
	default:
		return ConvertToSpriteScale( RADIUS, RADIUS_TO_SCALE );
	}
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostOnDestroy( )
{
	if( IsCrystal( ) )
	{
		GameState::SetCrystal( nullptr, true );
		g_GameSystem->m_crystalDestroyed = true;
		g_GameSystem->HostWipeGame( );
	}

	Ship::HostOnDestroy( );
}


//-------------------------------------------------------------------------------------------------
void AllyShip::ClientOnDestroy( )
{
	if( IsCrystal( ) )
	{
		GameState::SetCrystal( nullptr, false );
		g_GameSystem->m_crystalDestroyed = true;
	}
}


//-------------------------------------------------------------------------------------------------
void AllyShip::ApplyControlInput( )
{
	//Host Input
	if( m_isHostObject )
	{
		HostApplyAIInput( );
	}
}


//-------------------------------------------------------------------------------------------------
int AllyShip::GetMaxHealth( ) const
{
	switch( m_allyType )
	{
	case eAllyType_CRYSTAL:
		return CRYSTAL_MAX_HEALTH;
	default:
		return MAX_HEALTH;
	}
}


//-------------------------------------------------------------------------------------------------
int AllyShip::GetMaxShield( ) const
{
	switch( m_allyType )
	{
	case eAllyType_CRYSTAL:
		return CRYSTAL_MAX_SHIELD;
	default:
		return MAX_SHIELD;
	}
}


//-------------------------------------------------------------------------------------------------
int AllyShip::GetMaxEnergy( ) const
{
	switch( m_allyType )
	{
	case eAllyType_CRYSTAL:
		return CRYSTAL_MAX_ENERGY;
	default:
		return MAX_ENERGY;
	}
}


//-------------------------------------------------------------------------------------------------
float AllyShip::GetMaxSpeed( ) const
{
	//Speed based on how slow your are
	if( g_GameSystem->IsHost( ) )
	{
		if( HostIsSlowed( ) )
		{
			return SPEED * m_slowAmount;
		}
	}
	return SPEED;
}


//-------------------------------------------------------------------------------------------------
float AllyShip::GetFireRate( ) const
{
	switch( m_allyType )
	{
	case eAllyType_GUARD:
		return FIRE_RATE_SECONDS;
	default:
		return 0.f;
	}
}


//-------------------------------------------------------------------------------------------------
uint8_t AllyShip::GetSummoningBarValue( ) const
{
	return 1U;
}


//-------------------------------------------------------------------------------------------------
Vector2f AllyShip::GetHomeLocation( ) const
{
	return ShipSpawner::GetLocation( m_spawnerID );
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostHealDamage( uint16_t incomingHeal )
{
	m_health += incomingHeal;
	if( m_health > AllyShip::CRYSTAL_MAX_HEALTH )
	{
		m_health = AllyShip::CRYSTAL_MAX_HEALTH;
	}
}


//-------------------------------------------------------------------------------------------------
bool AllyShip::IsCrystal( ) const
{
	return m_allyType == eAllyType_CRYSTAL;
}


//-------------------------------------------------------------------------------------------------
void AllyShip::UpdateTimers( )
{
	m_aiTimer += Time::DELTA_SECONDS;
	m_shieldRegenTimer += Time::DELTA_SECONDS;
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostApplyAIInput( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	//Movement
	switch( m_allyType )
	{
	case eAllyType_GUARD:

		if( !HasTarget( ) )
		{
			HostAcquireNewTarget( );
		}

		//Move towards target
		if( HasTarget( ) )
		{
			HostTurnTowardsTarget( );
			HostMoveTowardsSafeZone( );
		}

		//Just keep swimming
		else
		{
			HostMoveTowardsWanderTarget( );
		}

		//Attack
		if( CanFire( ) && HasTarget( ) )
		{
			HostFireWeapon( );
			ResetFireCooldown( );
		}

		break;
	}
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostAcquireNewTarget( )
{
	Ship * closestShip = nullptr;
	float distanceToShip = g_GameSystem->HostGetClosestEvilShip( m_position, &closestShip );
	if( closestShip )
	{
		if( distanceToShip < SIGHT_RADIUS )
		{
			m_targetShip = closestShip;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostTurnTowardsTarget( )
{
	Vector2f vectorTowardsShip = m_targetShip->m_position - m_position;
	float targetDegrees = Atan2Degrees( vectorTowardsShip );
	float currentDegrees = m_rotationDegrees;
	float turnAmount = ShortestSignedAngularDistance( currentDegrees, targetDegrees );
	float turnAmountLimited = Clamp( turnAmount, -TURN_SPEED * Time::DELTA_SECONDS, TURN_SPEED * Time::DELTA_SECONDS );
	m_rotationDegrees += turnAmountLimited;
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostMoveTowardsSafeZone( )
{
	float distanceToShip = DistanceBetweenPoints( m_position, m_targetShip->m_position );

	//Lose sight of target
	if( distanceToShip > SIGHT_RADIUS )
	{
		m_targetShip = nullptr;
	}

	//Too far
	if( distanceToShip > SAFE_DISTANCE + SAFE_RADIUS / 2.f )
	{
		m_velocity = Lerp( m_velocity, UnitVectorFromDegrees( m_rotationDegrees ) * SPEED, SPEED_DECAY_RATE );
	}

	//Too close
	else if( distanceToShip < SAFE_DISTANCE - SAFE_RADIUS / 2.f )
	{
		m_velocity = Lerp( m_velocity, -UnitVectorFromDegrees( m_rotationDegrees ) * SPEED, SPEED_DECAY_RATE );
	}

	//Settle
	else
	{
		m_velocity = m_velocity * ( 1.f - SPEED_DECAY_RATE );
	}
}


//-------------------------------------------------------------------------------------------------
void AllyShip::HostMoveTowardsWanderTarget( )
{
	float distance = DistanceBetweenPoints( m_position, m_wanderTarget );
	if( distance < WANDER_THRESHOLD )
	{
		m_wanderTarget = GetHomeLocation( ) + RandomUnitVectorCircle( ) * MAX_WANDER_DISTANCE;
	}

	Vector2f vectorTowardsTarget = m_wanderTarget - m_position;
	float targetDegrees = Atan2Degrees( vectorTowardsTarget );
	float currentDegrees = m_rotationDegrees;
	float turnAmount = ShortestSignedAngularDistance( currentDegrees, targetDegrees );
	float turnAmountLimited = Clamp( turnAmount, -TURN_SPEED * Time::DELTA_SECONDS, TURN_SPEED * Time::DELTA_SECONDS );
	m_rotationDegrees += turnAmountLimited;

	m_velocity = Lerp( m_velocity, UnitVectorFromDegrees( m_rotationDegrees ) * SPEED, SPEED_DECAY_RATE );
}


//-------------------------------------------------------------------------------------------------
bool AllyShip::HasTarget( ) const
{
	return m_targetShip != nullptr;
}


//-------------------------------------------------------------------------------------------------
void AllyShip::CreateOrUpdateCrystalBubble( )
{
	if( m_crystalBubble == nullptr )
	{
		m_crystalBubble = SpriteGameRenderer::Create( "circle", LAYER_CRYSTAL - 1 );
	}
	m_crystalBubble->SetScale( GetSpriteScale( ) * 2.f );
	m_crystalBubble->SetColor( Color( 128, 128, 255, 128 ) );
}