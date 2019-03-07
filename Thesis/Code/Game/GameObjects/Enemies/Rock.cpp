#include "Game/GameObjects/Enemies/Rock.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * Rock::NAME = "Rock";
STATIC float const Rock::NET_SYNC_PERCENT = 1.0f;
STATIC float const Rock::RADIUS = 0.5f;
STATIC float const Rock::ROCK_RADIUS_TO_SCALE = 2.8f;
STATIC float const Rock::MAX_SPIN_SPEED = 25.f;
STATIC int const Rock::MAX_HEALTH = 40;
STATIC GameObjectRegistration Rock::s_RockMessageRegistration( eNetGameObjectType_ROCK, &Rock::MessageCreation );
STATIC GameObjectRegistration Rock::s_RockXMLRegistration( NAME, &Rock::XMLCreation );


//-------------------------------------------------------------------------------------------------
STATIC GameObject * Rock::MessageCreation( NetSender const & sender, NetMessage const & message )
{
	return new Rock( sender, message );
}


//-------------------------------------------------------------------------------------------------
STATIC GameObject * Rock::XMLCreation( XMLNode const & node )
{
	return new Rock( node );
}


//-------------------------------------------------------------------------------------------------
Rock::Rock( NetSender const & sender, NetMessage const & message )
	: Rock( false )
{
	UpdateFromMessage( sender, message );
	m_position = m_netPosition;
	m_health = m_netHealth;
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Rock::WriteToMessage( NetMessage * out_message )
{
	Game::WriteCompressedPosition( out_message, m_position );
	Game::WriteUncompressedUint16( out_message, m_health );
}


//-------------------------------------------------------------------------------------------------
//Only Clients would do this, update their copy of the net data
void Rock::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Game::ReadCompressedPosition( message, &m_netPosition );
	Game::ReadUncompressedUint16( message, &m_netHealth );

	//I know this isn't currently moving, but in case I want it to move in the future
	//Predict current location
	UpdateNetData( GetElapsedGameTime( sender ) );
	MessageRecieved( );
}


//-------------------------------------------------------------------------------------------------
Rock::Rock( XMLNode const & node )
	: Rock( true )
{
	UpdateFromXMLNode( node );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Rock::WriteToXMLNode( XMLNode * out_xmlNode )
{
	XMLNode node = out_xmlNode->addChild( NAME );
	node.addAttribute( STRING_POSITION, Stringf( "%f,%f", m_position.x, m_position.y ).c_str( ) );
	node.addAttribute( STRING_HEALTH, Stringf( "%u", m_health ).c_str( ) );

	GameObject::WriteToXMLNode( &node );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Rock::UpdateFromXMLNode( XMLNode const & node )
{
	m_position = ReadXMLAttribute( node, STRING_POSITION, m_position );
	m_health = ReadXMLAttribute( node, STRING_HEALTH, m_health );
}


//-------------------------------------------------------------------------------------------------
Rock::Rock( bool hostObject )
	: GameObject( INVALID_INDEX, eNetGameObjectType_ROCK, hostObject )
	, m_health( MAX_HEALTH )
	, m_netHealth( MAX_HEALTH )
	, m_spinSpeed( 0.f )
	, m_randomSpriteNum( 0 )
{
	//(-max_spin) to (max_spin)
	m_spinSpeed = RandomFloat( 2.f * MAX_SPIN_SPEED ) - MAX_SPIN_SPEED;

	//Give rock a random sprite (0 - 3)
	m_randomSpriteNum = RandomInt( 4 );
}


//-------------------------------------------------------------------------------------------------
Rock::~Rock( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void Rock::Update( )
{
	//Client Update
	if( !m_isHostObject )
	{
		m_health = m_netHealth;
	}

	//Fudge with net data to make them spin
	//m_netRotationDegrees += Time::DELTA_SECONDS * m_spinSpeed;
	
	GameObject::Update( );
}


//-------------------------------------------------------------------------------------------------
void Rock::HostHandleCollision( GameObject * )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
bool Rock::HostIsAlive( ) const
{
	return m_health > 0;
}


//-------------------------------------------------------------------------------------------------
char const * Rock::GetEntityName( ) const
{
	return NAME;
}


//-------------------------------------------------------------------------------------------------
std::string Rock::GetSpriteID( ) const
{
	switch( m_randomSpriteNum )
	{
	case 0:
		return "asteroid1";
	case 1:
		return "asteroid2";
	case 2:
		return "asteroid3";
	case 3:
		return "asteroid4";
	default:
		return SPRITE_ID_ERROR;
	}
}


//-------------------------------------------------------------------------------------------------
float Rock::GetNetSyncPercent( ) const
{
	return NET_SYNC_PERCENT;
}


//-------------------------------------------------------------------------------------------------
float Rock::GetPhysicsRadius( ) const
{
	return RADIUS;
}


//-------------------------------------------------------------------------------------------------
float Rock::GetSpriteScale( ) const
{
	return ConvertToSpriteScale( RADIUS, ROCK_RADIUS_TO_SCALE );
}


//-------------------------------------------------------------------------------------------------
void Rock::HostOnDestroy( )
{
	if( !g_GameSystem->IsHost( ) )
	{
		return;
	}

	//When a rock dies, spawn a rock
	//Always same number of rocks in the world
	g_GameSystem->HostSpawnRock( );

	//Spawn a random amount of money (1 - 5)
	uint8_t cubeAmount = (uint8_t) RandomInt( 1, 6 );
	g_GameSystem->HostSpawnCubes( m_position, cubeAmount );
	//uint16_t weaponCode = Item::GenerateRandomWeapon( 1U );
	//g_GameSystem->HostSpawnItem( m_position, weaponCode );
}


//-------------------------------------------------------------------------------------------------
void Rock::ClientOnDestroy( )
{
	g_ParticleEngine->PlayOnce( "rockBurst", LAYER_FX, m_position );
}


//-------------------------------------------------------------------------------------------------
void Rock::HostTakeDamage( uint16_t incomingDamage )
{
	if( incomingDamage > m_health )
	{
		m_health = 0;
	}
	else
	{
		m_health -= incomingDamage;
	}
}


//-------------------------------------------------------------------------------------------------
int Rock::GetMaxHealth( ) const
{
	return MAX_HEALTH;
}