#include "Game/GameObjects/Items/Pickup.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/GameObjects/Items/Item.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * Pickup::NAME = "Pickup";
STATIC float const Pickup::NET_SYNC_PERCENT = 1.0f; //100%
STATIC float const Pickup::RADIUS = 0.2f;
STATIC float const Pickup::MAX_AGE = 30.0f;
STATIC GameObjectRegistration Pickup::s_PickupXMLRegistration( NAME, &Pickup::XMLCreation );
STATIC GameObjectRegistration Pickup::s_PickupMessageRegistration( eNetGameObjectType_PICKUP, &Pickup::MessageCreation );


//-------------------------------------------------------------------------------------------------
STATIC GameObject * Pickup::MessageCreation( NetSender const & sender, NetMessage const & message )
{
	return new Pickup( sender, message );
}


//-------------------------------------------------------------------------------------------------
STATIC GameObject * Pickup::XMLCreation( XMLNode const & node )
{
	return new Pickup( node );
}


//-------------------------------------------------------------------------------------------------
Pickup::Pickup( NetSender const & sender, NetMessage const & message )
	: Pickup( eItemType_ERROR, false )
{
	UpdateFromMessage( sender, message );
	m_position = m_netPosition;
	m_itemCode = m_netItemCode;
}

//-------------------------------------------------------------------------------------------------
//Host Only
void Pickup::WriteToMessage( NetMessage * out_message )
{
	//(8) + (2) = 10 bytes
	//(4) + (2) = 6 bytes
	Game::WriteCompressedPosition( out_message, m_position );
	Game::WriteUncompressedUint16( out_message, m_itemCode );
}


//-------------------------------------------------------------------------------------------------
//Only Clients would do this, update their copy of the net data
void Pickup::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Game::ReadCompressedPosition( message, &m_netPosition );
	Game::ReadUncompressedUint16( message, &m_netItemCode );

	//I know this isn't currently moving, but in case I want it to move in the future
	//Predict current location
	UpdateNetData( GetElapsedGameTime( sender ) );
	MessageRecieved( );
}


//-------------------------------------------------------------------------------------------------
Pickup::Pickup( XMLNode const & node )
	: Pickup( eItemType_ERROR, true )
{
	UpdateFromXMLNode( node );
}



//-------------------------------------------------------------------------------------------------
//Host Only
void Pickup::WriteToXMLNode( XMLNode * out_xmlNode )
{
	XMLNode node = out_xmlNode->addChild( NAME );
	node.addAttribute( STRING_POSITION, Stringf( "%f,%f", m_position.x, m_position.y ).c_str( ) );
	node.addAttribute( STRING_ITEM_CODE, Stringf( "%u", m_itemCode ).c_str( ) );

	GameObject::WriteToXMLNode( &node );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Pickup::UpdateFromXMLNode( XMLNode const & node )
{
	m_position = ReadXMLAttribute( node, STRING_POSITION, m_position );
	m_itemCode = ReadXMLAttribute<uint16_t>( node, STRING_ITEM_CODE, m_itemCode );
}


//-------------------------------------------------------------------------------------------------
Pickup::Pickup( eItemType const & itemType, bool hostObject )
	: GameObject( INVALID_INDEX, eNetGameObjectType_PICKUP, hostObject )
	, m_itemCode( Item::GenerateRandomItem( itemType ) )
	, m_netItemCode( m_itemCode )
	, m_taken( false )
	, m_age( 0.0f )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Pickup::Pickup( uint16_t itemCode, bool hostObject )
	: GameObject( INVALID_INDEX, eNetGameObjectType_PICKUP, hostObject )
	, m_itemCode( itemCode )
	, m_netItemCode( itemCode )
	, m_taken( false )
	, m_age( 0.0f )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Pickup::~Pickup( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void Pickup::Update( )
{
	//Host Update
	if( m_isHostObject )
	{
		m_age += Time::DELTA_SECONDS;
	}

	//Client Update
	else if( !m_isHostObject )
	{
		m_itemCode = m_netItemCode;
	}

	GameObject::Update( );
}


//-------------------------------------------------------------------------------------------------
void Pickup::HostHandleCollision( GameObject * )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
bool Pickup::HostIsAlive( ) const
{
	return !m_taken && m_age < MAX_AGE;
}


//-------------------------------------------------------------------------------------------------
char const * Pickup::GetEntityName( ) const
{
	return NAME;
}


//-------------------------------------------------------------------------------------------------
std::string Pickup::GetSpriteID( ) const
{
	return Item::ParseIconSprite( m_itemCode );
}


//-------------------------------------------------------------------------------------------------
float Pickup::GetNetSyncPercent( ) const
{
	return NET_SYNC_PERCENT;
}


//-------------------------------------------------------------------------------------------------
float Pickup::GetPhysicsRadius( ) const
{
	return RADIUS;
}


//-------------------------------------------------------------------------------------------------
float Pickup::GetSpriteScale( ) const
{
	return ConvertToSpriteScale( RADIUS, RADIUS_TO_SCALE );
}