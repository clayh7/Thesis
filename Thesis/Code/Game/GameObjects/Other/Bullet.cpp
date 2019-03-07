#include "Game/GameObjects/Other/Bullet.hpp"

#include "Engine/RenderSystem/SpriteRenderSystem/ParticleSystem.hpp"
#include "Engine/AudioSystem/Audio.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/General/Game.hpp"
#include "Game/GameObjects/Player/PlayerShip.hpp"
#include "Game/GameObjects/Enemies/EnemyShip.hpp"
#include "Game/GameObjects/Enemies/Rock.hpp"
#include "Game/GameObjects/Allies/AllyShip.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * Bullet::NAME = "Bullet";
STATIC float const Bullet::NET_SYNC_PERCENT = 0.10f;
STATIC float const Bullet::SPEED = 6.0f;
STATIC float const Bullet::RADIUS = 0.2f;
STATIC float const Bullet::BULLET_RADIUS_TO_SCALE = 6.f;
STATIC float const Bullet::LIFETIME = 10.f;
STATIC float const Bullet::RADIUS_PER_WEAPON_AOE = 0.75f;

STATIC GameObjectRegistration Bullet::s_BulletMessageRegistration( eNetGameObjectType_BULLET, &Bullet::MessageCreation );


//-------------------------------------------------------------------------------------------------
STATIC GameObject * Bullet::MessageCreation( NetSender const & sender, NetMessage const & message )
{
	return new Bullet( sender, message );
}


//-------------------------------------------------------------------------------------------------
Bullet::Bullet( NetSender const & sender, NetMessage const & message )
	: Bullet( INVALID_INDEX, Item::INVALID_ITEM_CODE, false )
{
	UpdateFromMessage( sender, message );
	m_position = m_netPosition;
	m_velocity = m_netVelocity;
	m_rotationDegrees = m_netRotationDegrees;
	m_itemCode = m_netItemCode;
}


//-------------------------------------------------------------------------------------------------
void Bullet::WriteToMessage( NetMessage * out_message )
{
	//(1) + (8) + (8) + (4) + (2) = 23 bytes
	//(1) + (4) + (4) + (1) + (2) = 12 bytes
	Game::WriteUncompressedUint8( out_message, m_playerIndex );
	Game::WriteCompressedPosition( out_message, m_position );
	Game::WriteCompressedVelocity( out_message, m_velocity );
	Game::WriteCompressedRotation( out_message, m_rotationDegrees );
	Game::WriteUncompressedUint16( out_message, m_itemCode );
}


//-------------------------------------------------------------------------------------------------
void Bullet::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Game::ReadUncompressedUint8( message, &m_playerIndex );
	Game::ReadCompressedPosition( message, &m_netPosition );
	Game::ReadCompressedVelocity( message, &m_netVelocity );
	Game::ReadCompressedRotation( message, &m_netRotationDegrees );
	Game::ReadUncompressedUint16( message, &m_netItemCode );

	//Predict current location
	UpdateNetData( GetElapsedGameTime( sender ) );
	MessageRecieved( );
}


//-------------------------------------------------------------------------------------------------
void Bullet::WriteToXMLNode( XMLNode * )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void Bullet::UpdateFromXMLNode( XMLNode const & )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Bullet::Bullet( byte_t playerIndex, uint16_t itemCode, bool hostObject )
	: GameObject( playerIndex, eNetGameObjectType_BULLET, hostObject )
	, m_itemCode( itemCode )
	, m_netItemCode( itemCode )
	, m_age( LIFETIME )
	, m_trailEffect( nullptr )
{
	//Make this for clients
	if( !m_isHostObject )
	{
		m_trailEffect = g_ParticleEngine->Create( "trail", LAYER_FX, m_position );
	}
}


//-------------------------------------------------------------------------------------------------
Bullet::~Bullet( )
{
	if( m_trailEffect != nullptr )
	{
		m_trailEffect->Destroy( );
	}
}


//-------------------------------------------------------------------------------------------------
void Bullet::Update( )
{
	//Host Update
	if( m_isHostObject )
	{
		// Decay age
		m_age -= Time::DELTA_SECONDS;
	}

	//Client Update
	else if( !m_isHostObject )
	{
		m_itemCode = m_netItemCode;
	}

	GameObject::Update( );

	if( m_trailEffect )
	{
		m_trailEffect->SetSpawnPosition( m_position );
	}
}


//-------------------------------------------------------------------------------------------------
void Bullet::HostHandleCollision( GameObject * gameObject )
{
	// Don't hurt self with friendly bullets
	if( gameObject->m_playerIndex == m_playerIndex )
	{
		return;
	}

	//Determine if the bullet came from an evil source |is an NPC
	bool isEvil = false;
	bool isNPC = false;

	//If it's an enemy bullet
	if( m_playerIndex == ENEMY_PLAYER_INDEX )
	{
		isEvil = true;
		isNPC = true;
	}
	else if( m_playerIndex == ALLY_PLAYER_INDEX )
	{
		isNPC = true;
	}

	//Or if it's an "enemy" player
	Player * owningPlayer = g_GameSystem->HostGetPlayer( m_playerIndex );
	if( owningPlayer )
	{
		if( owningPlayer->IsEvilClass( ) )
		{
			isEvil = true;
		}
	}

	// Collision with Ships
	if( gameObject->m_type == eNetGameObjectType_PLAYERSHIP )
	{
		PlayerShip * hitShip = (PlayerShip*) gameObject;
		if( isEvil && isNPC && hitShip->HostIsEvil( ) )
		{
			//No Damage
		}
		else if( !isEvil && isNPC && !hitShip->HostIsEvil( ) )
		{
			//No Damage
		}
		else
		{
			hitShip->HostTakeDamage( GetDamage( ), m_playerIndex, m_itemCode );
			HostKillBullet( );
		}
	}

	// Collision with Enemies
	else if( gameObject->m_type == eNetGameObjectType_ENEMYSHIP )
	{
		if( !isEvil )
		{
			EnemyShip * hitEnemy = (EnemyShip*) gameObject;
			hitEnemy->HostTakeDamage( GetDamage( ), m_playerIndex, m_itemCode );
			HostKillBullet( );
		}
	}

	// Collision with Rocks
	else if( gameObject->m_type == eNetGameObjectType_ROCK )
	{
		Rock * hitRock = (Rock*) gameObject;
		hitRock->HostTakeDamage( GetDamage( ) );
		HostKillBullet( );
	}

	// Collision with Crystal
	else if( gameObject->m_type == eNetGameObjectType_ALLYSHIP )
	{
		//Hurt the crystal
		if( isEvil )
		{
			AllyShip * hitCrystal = (AllyShip*) gameObject;
			hitCrystal->HostTakeDamage( GetDamage( ), m_playerIndex, m_itemCode );
			HostKillBullet( );
		}
	}
}


//-------------------------------------------------------------------------------------------------
bool Bullet::HostIsAlive( ) const
{
	return m_age > 0.f;
}


//-------------------------------------------------------------------------------------------------
char const * Bullet::GetEntityName( ) const
{
	return NAME;
}


//-------------------------------------------------------------------------------------------------
std::string Bullet::GetSpriteID( ) const
{
	return Item::ParseBulletSprite( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
float Bullet::GetNetSyncPercent( ) const
{
	return NET_SYNC_PERCENT;
}


//-------------------------------------------------------------------------------------------------
float Bullet::GetPhysicsRadius( ) const
{
	return RADIUS;
}


//-------------------------------------------------------------------------------------------------
float Bullet::GetSpriteScale( ) const
{
	return ConvertToSpriteScale( RADIUS, BULLET_RADIUS_TO_SCALE );
}


//-------------------------------------------------------------------------------------------------
void Bullet::ClientOnDestroy( )
{
	eItemType weaponType = Item::ParseItemType( m_itemCode );
	if( weaponType == eItemType_WEAPON_BOMBER || weaponType == eItemType_WEAPON_ROCKET )
	{
		g_ParticleEngine->PlayOnce( "explosion", LAYER_FX, m_position );
	}
	g_ParticleEngine->PlayOnce( "bulletPop", LAYER_FX, m_position );
	float attenuation = g_GameSystem->GetAudioAttenuationFromCamera( m_position );
	SoundID hitID = GetRandomHitSound( );
	g_AudioSystem->PlayAudio( hitID, attenuation );
}


//-------------------------------------------------------------------------------------------------
uint16_t Bullet::GetDamage( )
{
	//return 0;
	return Item::ParseDamage( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
uint16_t Bullet::GetEnergyCost( )
{
	return Item::ParseEnergyCost( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
float Bullet::GetFireRate( )
{
	return Item::ParseFireRate( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
uint8_t Bullet::GetSizeAttribute( )
{
	return Item::ParseSizeAttribute( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
void Bullet::HostKillBullet( )
{
	m_age = 0.f;

	eItemType weaponType = Item::ParseItemType( m_itemCode );
	if( weaponType == eItemType_WEAPON_BOMBER || weaponType == eItemType_WEAPON_ROCKET )
	{
		uint8_t weaponSize = Item::ParseSizeAttribute( m_itemCode );
		float areaRadius = (float) weaponSize * Bullet::RADIUS_PER_WEAPON_AOE;
		g_GameSystem->HostDealAreaDamage( m_position, areaRadius, Item::ParseDamage( m_itemCode ), m_playerIndex );
	}
}
