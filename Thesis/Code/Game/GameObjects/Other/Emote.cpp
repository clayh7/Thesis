#include "Game/GameObjects/Other/Emote.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * Emote::NAME = "Emote";
STATIC float const Emote::NET_SYNC_PERCENT = 1.0f;
STATIC float const Emote::RADIUS = 0.5f;
STATIC float const Emote::MAX_AGE = 5.0f;
STATIC GameObjectRegistration Emote::s_EmoteXMLRegistration( NAME, &Emote::XMLCreation );
STATIC GameObjectRegistration Emote::s_EmoteMessageRegistration( eNetGameObjectType_EMOTE, &Emote::MessageCreation );


//-------------------------------------------------------------------------------------------------
STATIC GameObject * Emote::MessageCreation( NetSender const & sender, NetMessage const & message )
{
	return new Emote( sender, message );
}


//-------------------------------------------------------------------------------------------------
STATIC GameObject * Emote::XMLCreation( XMLNode const & node )
{
	return new Emote( node );
}


//-------------------------------------------------------------------------------------------------
Emote::Emote( NetSender const & sender, NetMessage const & message )
	: Emote( eEmoteType_INVALID, false )
{
	UpdateFromMessage( sender, message );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Emote::WriteToMessage( NetMessage * out_message )
{
	//(8) + (1) = 9 bytes
	//(4) + (1) = 5 bytes
	Game::WriteUncompressedUint8( out_message, m_emoteType );
	Game::WriteCompressedPosition( out_message, m_position );
}


//-------------------------------------------------------------------------------------------------
//Only Clients would do this, update their copy of the net data
void Emote::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Game::ReadUncompressedUint8( message, (uint8_t*) &m_emoteType );
	Game::ReadCompressedPosition( message, &m_netPosition );

	//I know this isn't currently moving, but in case I want it to move in the future
	//Predict current location
	UpdateNetData( GetElapsedGameTime( sender ) );
	MessageRecieved( );
}


//-------------------------------------------------------------------------------------------------
Emote::Emote( XMLNode const & node )
	: Emote( eEmoteType_INVALID, true )
{
	UpdateFromXMLNode( node );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Emote::WriteToXMLNode( XMLNode * out_xmlNode )
{
	XMLNode node = out_xmlNode->addChild( NAME );
	node.addAttribute( STRING_EMOTE_TYPE, Stringf( "%u", m_emoteType ).c_str( ) );
	node.addAttribute( STRING_POSITION, Stringf( "%f,%f", m_position.x, m_position.y ).c_str( ) );
	node.addAttribute( STRING_ROTATION, Stringf( "%f", m_rotationDegrees ).c_str( ) );

	GameObject::WriteToXMLNode( &node );
}


//-------------------------------------------------------------------------------------------------
//Host Only
void Emote::UpdateFromXMLNode( XMLNode const & node )
{
	m_emoteType = (eEmoteType)ReadXMLAttribute( node, STRING_EMOTE_TYPE, (uint8_t) m_emoteType );
	m_position = ReadXMLAttribute( node, STRING_POSITION, m_position );
	m_rotationDegrees = ReadXMLAttribute( node, STRING_ROTATION, m_rotationDegrees );
}


//-------------------------------------------------------------------------------------------------
Emote::Emote( eEmoteType emoteType, bool hostObject )
	: GameObject( INVALID_INDEX, eNetGameObjectType_EMOTE, hostObject )
	, m_emoteType( emoteType )
	, m_age( 0.f )
{
	m_sprite->SetLayer( LAYER_EMOTE );
}


//-------------------------------------------------------------------------------------------------
Emote::~Emote( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void Emote::Update( )
{
	m_age += Time::DELTA_SECONDS;
	m_sprite->SetColor( GetSpriteColor( ) );

	//Billboard to camera
	m_netRotationDegrees = g_GameSystem->m_cameraRotation + 90.f;
	m_rotationDegrees = g_GameSystem->m_cameraRotation + 90.f;

	GameObject::Update( );
}


//-------------------------------------------------------------------------------------------------
void Emote::HostHandleCollision( GameObject * )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
bool Emote::HostIsAlive( ) const
{
	return m_age < MAX_AGE;
}


//-------------------------------------------------------------------------------------------------
char const * Emote::GetEntityName( ) const
{
	return NAME;
}


//-------------------------------------------------------------------------------------------------
std::string Emote::GetSpriteID( ) const
{
	switch( m_emoteType )
	{
	case eEmoteType_HAPPY:
		return "emoteHappy";
	case eEmoteType_SAD:
		return "emoteSad";
	case eEmoteType_ANGRY:
		return "emoteAngry";
	case eEmoteType_WHOOPS:
		return "emoteWhoops";
	case eEmoteType_HELP:
		return "emoteHelp";
	default:
		return "error";
	}
}


//-------------------------------------------------------------------------------------------------
float Emote::GetNetSyncPercent( ) const
{
	return NET_SYNC_PERCENT;
}


//-------------------------------------------------------------------------------------------------
float Emote::GetPhysicsRadius( ) const
{
	return RADIUS;
}


//-------------------------------------------------------------------------------------------------
float Emote::GetSpriteScale( ) const
{
	float radius = GetPhysicsRadius( );
	return ConvertToSpriteScale( radius, RADIUS_TO_SCALE );
}


//-------------------------------------------------------------------------------------------------
Color Emote::GetSpriteColor( ) const
{
	Color currentColor = m_sprite->m_color;
	currentColor.a = (uint8_t) Clamp( (int) ( ( 1.0f - ( m_age / MAX_AGE ) ) * 256.f ), 0, 256 );
	return currentColor;
}
