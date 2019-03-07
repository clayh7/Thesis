#include "Game/GameObjects/Enemies/EnemyShip.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/General/Game.hpp"
#include "Game/GameObjects/Enemies/ShipSpawner.hpp"
#include "Game/GameObjects/Enemies/Rock.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/GameObjects/Other/Bullet.hpp"
#include "Game/GameObjects/Other/Emote.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * EnemyShip::NAME = "EnemyShip";
STATIC float const EnemyShip::NET_SYNC_PERCENT = 0.10f;
STATIC float const EnemyShip::SPEED = 1.f;
STATIC float const EnemyShip::SPEED_DECAY_RATE = 0.05f;
STATIC float const EnemyShip::TURN_SPEED = 45.f;
STATIC float const EnemyShip::RADIUS = 0.5f;
STATIC float const EnemyShip::SIGHT_RADIUS = 10.f;
STATIC float const EnemyShip::SAFE_DISTANCE = 4.0f;
STATIC float const EnemyShip::SAFE_RADIUS = 0.5f; //(safe zone Max - safe zone Min)
STATIC float const EnemyShip::FIRE_RATE_SECONDS = 1.5f;
STATIC uint8_t const EnemyShip::CUBES_DROPPED_PER_LEVEL = 5U;
STATIC float const EnemyShip::DROP_WEAPON_CHANCE = 0.3f;
STATIC float const EnemyShip::BOSS_RED_RADIUS = 2.f;
STATIC float const EnemyShip::BOSS_RED_FIRE_RATE_SECONDS = 0.4f;
STATIC float const EnemyShip::BOSS_RED_TURN_SPEED = 20.f;
STATIC float const EnemyShip::MAX_WANDER_DISTANCE = 7.5f;
STATIC float const EnemyShip::WANDER_THRESHOLD = 1.f;
STATIC int const EnemyShip::BASE_HEALTH = 75;
STATIC int const EnemyShip::HEALTH_PER_LEVEL = 20;
STATIC float const EnemyShip::SHIELD_REGEN_COOLDOWN = 1.0f;
STATIC int const EnemyShip::BASE_SHIELD = 50;
STATIC int const EnemyShip::SHIELD_PER_LEVEL = 20;
STATIC int const EnemyShip::MAX_ENERGY = 50;
STATIC uint16_t EnemyShip::CRYSTAL_HEAL_REWARD = 1000U;
STATIC GameObjectRegistration EnemyShip::s_EnemyXMLRegistration( NAME, &EnemyShip::XMLCreation );
STATIC GameObjectRegistration EnemyShip::s_EnemyMessageRegistration( eNetGameObjectType_ENEMYSHIP, &EnemyShip::MessageCreation );


//-------------------------------------------------------------------------------------------------
STATIC GameObject * EnemyShip::MessageCreation( NetSender const & sender, NetMessage const & message )
{
	return new EnemyShip( sender, message );
}


//-------------------------------------------------------------------------------------------------
STATIC GameObject * EnemyShip::XMLCreation( XMLNode const & node )
{
	return new EnemyShip( node );
}


//-------------------------------------------------------------------------------------------------
STATIC eEnemyType EnemyShip::ParseEnemyType( uint8_t enemyCode )
{
	uint8_t type = enemyCode >> 4;
	return (eEnemyType) type;
}


//-------------------------------------------------------------------------------------------------
STATIC uint8_t EnemyShip::ParseLevel( uint8_t enemyCode )
{
	uint8_t levelMask = 0x0F;
	uint8_t level = ( enemyCode & levelMask );
	return level;
}


//-------------------------------------------------------------------------------------------------
STATIC uint8_t EnemyShip::GenerateCode( eEnemyType type, uint8_t level )
{
	ASSERT_RECOVERABLE( level < 16, "Level must be 4 bits or less" );
	uint8_t code = type;
	code = code << 4;
	code = code | level;
	return code;
}


//-------------------------------------------------------------------------------------------------
EnemyShip::EnemyShip( NetSender const & sender, NetMessage const & message )
	: EnemyShip( eEnemyType_INVALID, 0U, false )
{
	UpdateFromMessage( sender, message );
	m_position = m_netPosition;
	m_velocity = m_netVelocity;
	m_rotationDegrees = m_netRotationDegrees;
	m_health = m_netHealth;
	m_enemyCode = m_netEnemyCode;

	m_wanderTarget = m_position;
}


//-------------------------------------------------------------------------------------------------
//Host Only
void EnemyShip::WriteToMessage( NetMessage * out_message )
{
	Ship::WriteToMessage( out_message );

	Game::WriteUncompressedUint8( out_message, m_enemyCode );
}


//-------------------------------------------------------------------------------------------------
//Only Clients would do this, update their copy of the net data
void EnemyShip::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Ship::UpdateFromMessage( sender, message );

	Game::ReadUncompressedUint8( message, &m_netEnemyCode );
}


//-------------------------------------------------------------------------------------------------
EnemyShip::EnemyShip( XMLNode const & node )
	: EnemyShip( eEnemyType_INVALID, 0U, true )
{
	UpdateFromXMLNode( node );

	m_wanderTarget = m_position;
}


//-------------------------------------------------------------------------------------------------
//Host Only
void EnemyShip::WriteToXMLNode( XMLNode * out_xmlNode )
{
	XMLNode node = out_xmlNode->addChild( NAME );
	eEnemyType type = GetEnemyType( );
	uint8_t level = GetLevel( );
	node.addAttribute( STRING_ENEMY_TYPE, Stringf( "%u", type ).c_str( ) );
	node.addAttribute( STRING_ENEMY_LEVEL, Stringf( "%u", level ).c_str( ) );

	Ship::WriteToXMLNode( &node );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void EnemyShip::UpdateFromXMLNode( XMLNode const & node )
{
	eEnemyType type = (eEnemyType) ReadXMLAttribute( node, STRING_ENEMY_TYPE, (uint8_t) eEnemyType_INVALID );
	uint8_t level = (uint8_t) ReadXMLAttribute( node, STRING_ENEMY_LEVEL, (uint8_t) 0U );
	m_enemyCode = EnemyShip::GenerateCode( type, level );

	Ship::UpdateFromXMLNode( node );
}


//-------------------------------------------------------------------------------------------------
EnemyShip::EnemyShip( eEnemyType const & enemyType, uint8_t level, bool hostObject )
	: Ship( ENEMY_PLAYER_INDEX, eNetGameObjectType_ENEMYSHIP, hostObject )
	, m_enemyCode( EnemyShip::GenerateCode( enemyType, level ) )
	, m_netEnemyCode( EnemyShip::GenerateCode( enemyType, level ) )
	, m_aiTimer( 0.f )
	, m_shieldRegenTimer( 0.f )
	, m_targetShip( nullptr )
	, m_wanderTarget( m_position )
{
	//Only if you're brand new (ie not invalid)
	eEnemyType type = GetEnemyType( );
	if( type != eEnemyType_INVALID )
	{
		m_health = (uint16_t) GetMaxHealth( );
		m_netHealth = (uint16_t) GetMaxHealth( );
		m_shield = (uint16_t) GetMaxShield( );
		m_netShield = (uint16_t) GetMaxShield( );
		m_energy = (uint16_t) GetMaxEnergy( );
		m_netEnergy = (uint16_t) GetMaxEnergy( );
	}
}


//-------------------------------------------------------------------------------------------------
EnemyShip::~EnemyShip( )
{
	ShipSpawner::RemoveFromSpawner( this );
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::Update( )
{
	UpdateTimers( );

	//Host Update
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
		m_enemyCode = m_netEnemyCode;
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
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::HostHandleCollision( GameObject * gameObject )
{
	// Don't collide with friendly objects
	if( gameObject->m_playerIndex == m_playerIndex )
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
void EnemyShip::HostFireWeapon( ) const
{
	eEnemyType type = GetEnemyType( );
	switch( type )
	{
	case eEnemyType_ARCHER:
		{
			//Get itemCode from enemyship
			uint16_t weaponCode = Item::GetItemCode( eItemType_WEAPON_PHASER, GetLevel( ), 0 );
			Bullet * firedBullet = new Bullet( m_playerIndex, weaponCode, m_isHostObject );
			//Move to the nose of the ship
			float radius = GetPhysicsRadius( );
			Vector2f bulletPosition = GetPositionInfront( BULLET_START_OFFSET * radius );
			firedBullet->m_position = bulletPosition;
			firedBullet->m_rotationDegrees = m_rotationDegrees;
			firedBullet->m_velocity = UnitVectorFromDegrees( m_rotationDegrees ) * Bullet::SPEED;
			g_GameSystem->HostCreateNetGameObject( firedBullet );
		}
		break;
	case eEnemyType_BOSS_RED:
		for( int bulletIndex = 0; bulletIndex < 4; ++bulletIndex )
		{
			float offsetRotation = m_rotationDegrees + 90.f * bulletIndex;
			//Get itemCode from enemyship
			uint16_t weaponCode = Item::GetItemCode( eItemType_WEAPON_PHASER, GetLevel( ), 0 );
			Bullet * firedBullet = new Bullet( m_playerIndex, weaponCode, m_isHostObject );
			//Move to the nose of the ship
			float radius = GetPhysicsRadius( );
			Vector2f bulletPosition = GetPositionInfront( BULLET_START_OFFSET * radius, offsetRotation );
			firedBullet->m_position = bulletPosition;
			firedBullet->m_rotationDegrees = WrapDegrees0to360( m_rotationDegrees + offsetRotation );
			firedBullet->m_velocity = UnitVectorFromDegrees( firedBullet->m_rotationDegrees ) * Bullet::SPEED;
			g_GameSystem->HostCreateNetGameObject( firedBullet );
		}
		break;
	}
}


//-------------------------------------------------------------------------------------------------
char const * EnemyShip::GetEntityName( ) const
{
	if( IsBoss( ) )
	{
		return "Ogre (Evil)";
	}
	else
	{
		return "Goblin (Evil)";
	}
}


//-------------------------------------------------------------------------------------------------
std::string EnemyShip::GetSpriteID( ) const
{
	eEnemyType type = GetEnemyType( );
	switch( type )
	{
	case eEnemyType_ARCHER:
		return "enemyArcher";
	case eEnemyType_BOSS_RED:
		return "enemyBossRed";
	default:
		return "error";
	}
}


//-------------------------------------------------------------------------------------------------
float EnemyShip::GetNetSyncPercent( ) const
{
	return NET_SYNC_PERCENT;
}


//-------------------------------------------------------------------------------------------------
float EnemyShip::GetPhysicsRadius( ) const
{
	eEnemyType type = GetEnemyType( );
	switch( type )
	{
	case eEnemyType_BOSS_RED:
		return BOSS_RED_RADIUS;
	default:
		return RADIUS;
	}
}


//-------------------------------------------------------------------------------------------------
float EnemyShip::GetSpriteScale( ) const
{
	eEnemyType type = GetEnemyType( );
	switch( type )
	{
	case eEnemyType_BOSS_RED:
		return ConvertToSpriteScale( BOSS_RED_RADIUS, RADIUS_TO_SCALE );
	default:
		return ConvertToSpriteScale( RADIUS, RADIUS_TO_SCALE );
	}
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::HostOnDestroy( )
{
	if( IsBoss( ) )
	{
		g_GameSystem->HostHealCrystal( CRYSTAL_HEAL_REWARD );
	}

	uint8_t level = GetLevel( );
	float randomDropRoll = RandomFloatZeroToOne( );
	//Spawn a random weapon
	if( randomDropRoll < DROP_WEAPON_CHANCE )
	{
		uint16_t weaponCode = Item::GenerateRandomWeapon( level );
		g_GameSystem->HostSpawnItem( m_position, weaponCode );
	}
	//Spawn a random amount of money
	else
	{
		byte_t cubeBonus = (byte_t) RandomInt( 1, 6 );
		byte_t cubeAmount = ( level * CUBES_DROPPED_PER_LEVEL ) + cubeBonus;
		g_GameSystem->HostSpawnCubes( m_position, cubeAmount );
	}

	Ship::HostOnDestroy( );
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::ClientOnDestroy( )
{
	Ship::ClientOnDestroy( );
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::UpdateTimers( )
{
	m_aiTimer += Time::DELTA_SECONDS;
	m_shieldRegenTimer += Time::DELTA_SECONDS;
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::ApplyControlInput( )
{
	//Host Input
	if( m_isHostObject )
	{
		HostApplyAIInput( );
	}
}


//-------------------------------------------------------------------------------------------------
int EnemyShip::GetMaxHealth( ) const
{
	return BASE_HEALTH + HEALTH_PER_LEVEL * GetLevel( );
}


//-------------------------------------------------------------------------------------------------
int EnemyShip::GetMaxShield( ) const
{
	return BASE_SHIELD + SHIELD_PER_LEVEL * GetLevel( );
}


//-------------------------------------------------------------------------------------------------
int EnemyShip::GetMaxEnergy( ) const
{
	return MAX_ENERGY;
}


//-------------------------------------------------------------------------------------------------
float EnemyShip::GetMaxSpeed( ) const
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
float EnemyShip::GetFireRate( ) const
{
	eEnemyType type = GetEnemyType( );
	switch( type )
	{
	case eEnemyType_BOSS_RED:
		return BOSS_RED_FIRE_RATE_SECONDS;
	default:
		return FIRE_RATE_SECONDS;
	}
}


//-------------------------------------------------------------------------------------------------
uint8_t EnemyShip::GetSummoningBarValue( ) const
{
	return 0U;
}


//-------------------------------------------------------------------------------------------------
Vector2f EnemyShip::GetHomeLocation( ) const
{
	return ShipSpawner::GetLocation( m_spawnerID );
}


//-------------------------------------------------------------------------------------------------
eEnemyType EnemyShip::GetEnemyType( ) const
{
	return ParseEnemyType( m_enemyCode );
}


//-------------------------------------------------------------------------------------------------
uint8_t EnemyShip::GetLevel( ) const
{
	return ParseLevel( m_enemyCode );
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::HostApplyAIInput( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	//Movement
	eEnemyType type = GetEnemyType( );
	switch( type )
	{
	case eEnemyType_BASIC:
		if( m_aiTimer > 5.f )
		{
			m_rotationDegrees += 90.f;
			m_aiTimer -= 5.f;
		}
		//m_speed = Lerp( m_speed, SPEED, SPEED_DECAY_RATE );
		break;
	case eEnemyType_ARCHER:
		//No target
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

		break;
	case eEnemyType_BOSS_RED:
		m_rotationDegrees += BOSS_RED_TURN_SPEED * Time::DELTA_SECONDS;
		if( !HasTarget( ) )
		{
			HostAcquireNewTarget( );
		}

		if( HasTarget( ) )
		{
			float distanceToShip = DistanceBetweenPoints( m_position, m_targetShip->m_position );

			//Lose sight of target
			if( distanceToShip > SIGHT_RADIUS )
			{
				m_targetShip = nullptr;
			}
		}
		break;
	}

	//Attack
	if( CanFire( ) && HasTarget( ) )
	{
		HostFireWeapon( );
		ResetFireCooldown( );
	}
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::HostAcquireNewTarget( )
{
	Ship * closestShip = nullptr;
	float distanceToShip = g_GameSystem->HostGetClosestGoodShip( m_position, &closestShip );
	if( closestShip )
	{
		if( distanceToShip < SIGHT_RADIUS )
		{
			m_targetShip = closestShip;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::HostTurnTowardsTarget( )
{
	Vector2f vectorTowardsShip = m_targetShip->m_position - m_position;
	float targetDegrees = Atan2Degrees( vectorTowardsShip );
	float currentDegrees = m_rotationDegrees;
	float turnAmount = ShortestSignedAngularDistance( currentDegrees, targetDegrees );
	float turnAmountLimited = Clamp( turnAmount, -TURN_SPEED * Time::DELTA_SECONDS, TURN_SPEED * Time::DELTA_SECONDS );
	m_rotationDegrees += turnAmountLimited;
}


//-------------------------------------------------------------------------------------------------
void EnemyShip::HostMoveTowardsSafeZone( )
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
void EnemyShip::HostMoveTowardsWanderTarget( )
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
bool EnemyShip::HasTarget( ) const
{
	return m_targetShip != nullptr;
}


//-------------------------------------------------------------------------------------------------
bool EnemyShip::IsBoss( ) const
{
	eEnemyType type = GetEnemyType( );
	return type == eEnemyType_BOSS_RED;
}
