#include "Game/GameObjects/GameObject.hpp"

#include "Engine/AudioSystem/Audio.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Game/General/Game.hpp"
#include "Game/GameObjects/Enemies/ShipSpawner.hpp"



//-------------------------------------------------------------------------------------------------
STATIC SoundID * GameObject::SOUND_HIT = new SoundID[SOUND_HIT_COUNT];
STATIC std::map<hash_t, GameObjectXMLCreationFunc*> * GameObject::s_registeredGameObjectsXML = nullptr;
STATIC std::map<eNetGameObjectType, GameObjectMessageCreationFunc*> * GameObject::s_registeredGameObjectsMessage = nullptr;
STATIC float const GameObject::MAX_TIMEOUT_SECONDS = 2.f;


//-------------------------------------------------------------------------------------------------
STATIC void GameObject::Initialize( )
{
	SOUND_HIT[0] = g_AudioSystem->CreateOrGetSound( SOUND_HIT1_FILE );
	SOUND_HIT[1] = g_AudioSystem->CreateOrGetSound( SOUND_HIT2_FILE );
	SOUND_HIT[2] = g_AudioSystem->CreateOrGetSound( SOUND_HIT3_FILE );
}


//-------------------------------------------------------------------------------------------------
STATIC GameObjectRegistration::GameObjectRegistration( std::string const & gameObjectName, GameObjectXMLCreationFunc * creationFunc )
{
	GameObject::RegisterGameObject( gameObjectName, creationFunc );
}


//-------------------------------------------------------------------------------------------------
STATIC GameObjectRegistration::GameObjectRegistration( eNetGameObjectType const & gameObjectType, GameObjectMessageCreationFunc * creationFunc )
{
	GameObject::RegisterGameObject( gameObjectType, creationFunc );
}


//-------------------------------------------------------------------------------------------------
STATIC void GameObject::RegisterGameObject( std::string const & gameObjectName, GameObjectXMLCreationFunc * creationFunc )
{
	//Create it if it does not exist
	if( !s_registeredGameObjectsXML )
	{
		s_registeredGameObjectsXML = new std::map<hash_t, GameObjectXMLCreationFunc*>( );
	}

	hash_t gameObjectHash = std::hash<std::string>{ }( gameObjectName );
	s_registeredGameObjectsXML->insert( std::pair<hash_t, GameObjectXMLCreationFunc*>( gameObjectHash, creationFunc ) );
}


//-------------------------------------------------------------------------------------------------
STATIC void GameObject::RegisterGameObject( eNetGameObjectType const & gameObjectType, GameObjectMessageCreationFunc * creationFunc )
{
	//Create it if it does not exist
	if( !s_registeredGameObjectsMessage )
	{
		s_registeredGameObjectsMessage = new std::map<eNetGameObjectType, GameObjectMessageCreationFunc*>( );
	}

	s_registeredGameObjectsMessage->insert( std::pair<eNetGameObjectType, GameObjectMessageCreationFunc*>( gameObjectType, creationFunc ) );
}


//-------------------------------------------------------------------------------------------------
GameObject * GameObject::CreateFromXMLNode( XMLNode const & node )
{
	std::string gameObjectName = node.getName( );
	hash_t gameObjectHash = std::hash<std::string>{ }( gameObjectName );
	auto foundGameObjectRegistration = s_registeredGameObjectsXML->find( gameObjectHash );
	if( foundGameObjectRegistration != s_registeredGameObjectsXML->end( ) )
	{
		return ( foundGameObjectRegistration->second )( node );
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
GameObject * GameObject::CreateFromNetMessage( eNetGameObjectType const & type, NetSender const & sender, NetMessage const & message )
{
	auto foundGameObjectRegistration = s_registeredGameObjectsMessage->find( type );
	if( foundGameObjectRegistration != s_registeredGameObjectsMessage->end( ) )
	{
		return ( foundGameObjectRegistration->second )( sender, message );
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
GameObject::GameObject( byte_t index, eNetGameObjectType const & type, bool hostObject )
	: m_isHostObject( hostObject )
	, m_sprite( nullptr )
	, m_netDebugSprite( nullptr )
	, m_timeOutTimer( 0.f )
	, m_playerIndex( index )
	, m_spawnerID( ShipSpawner::INVALID_SPAWNER_ID )
	, m_type( type )
	, m_position( Vector2f::ZERO )
	, m_netPosition( Vector2f::ZERO )
	, m_velocity( 0.f )
	, m_netVelocity( 0.f )
	, m_rotationDegrees( 0.f )
	, m_netRotationDegrees( 0.f )
{
	std::string spriteID = GetSpriteID( );
	m_sprite = SpriteGameRenderer::Create( spriteID );

	//For debugging host information
	if( g_GameSystem->m_isDedicatedServer )
	{
		m_sprite->SetEnabled( false );
		//m_sprite->SetColor( Game::HOST_COLOR );
		//SetVisibility( Game::SHOW_HOST );
	}
	else if( m_isHostObject )
	{
		m_sprite->SetColor( Game::HOST_COLOR );
		SetVisibility( Game::s_showHost );
	}

	//Local player collision debug
	if( Game::SHOW_COLLISION && !m_isHostObject )
	{
		m_netDebugSprite = SpriteGameRenderer::Create( "circle", LAYER_DEBUG_PLAYER );
		m_netDebugSprite->SetColor( Color( 0, 255, 0, 128 ) );
	}
}


//-------------------------------------------------------------------------------------------------
GameObject::GameObject( eNetGameObjectType const & type, bool hostObject )
	: GameObject( INVALID_INDEX, type, hostObject )
{
	// Nothing
}


//-------------------------------------------------------------------------------------------------
GameObject::~GameObject( )
{
	delete m_sprite;
	m_sprite = nullptr;

	delete m_netDebugSprite;
	m_netDebugSprite = nullptr;
}


//-------------------------------------------------------------------------------------------------
void GameObject::WriteToMessage( NetMessage * out_message )
{
	Game::WriteCompressedPosition( out_message, m_position );
	Game::WriteCompressedVelocity( out_message, m_velocity );
	Game::WriteCompressedRotation( out_message, m_rotationDegrees );
}


//-------------------------------------------------------------------------------------------------
void GameObject::UpdateFromMessage( NetSender const & sender, NetMessage const & message )
{
	Game::ReadCompressedPosition( message, &m_netPosition );
	Game::ReadCompressedVelocity( message, &m_velocity );
	Game::ReadCompressedRotation( message, &m_netRotationDegrees );

	//Move things to where they probably are on the host at this moment in time
	UpdateNetData( GetElapsedGameTime( sender ) );
	MessageRecieved( );
}


//-------------------------------------------------------------------------------------------------
void GameObject::WriteToXMLNode( XMLNode * out_xmlNode )
{
	if( m_spawnerID != ShipSpawner::INVALID_SPAWNER_ID )
	{
		out_xmlNode->addAttribute( "spawnerID", Stringf( "%u", m_spawnerID ).c_str( ) );
	}
}


//-------------------------------------------------------------------------------------------------
void GameObject::UpdateFromXMLNode( XMLNode const & node )
{
	m_spawnerID = ReadXMLAttribute( node, STRING_SPAWNER_ID, ShipSpawner::INVALID_SPAWNER_ID );
}


//-------------------------------------------------------------------------------------------------
void GameObject::Update( )
{
	//Host Update
	if( m_isHostObject )
	{
		UpdateLocalData( Time::DELTA_SECONDS );
	}

	//Client Update
	else if( !m_isHostObject )
	{
		UpdateLocalData( Time::DELTA_SECONDS );
		UpdateNetData( Time::DELTA_SECONDS );

		//Ease towards true host data
		m_velocity = m_netVelocity;
		m_rotationDegrees = LerpDegrees( m_rotationDegrees, m_netRotationDegrees, GetNetSyncPercent( ) );
		m_position = Lerp( m_position, m_netPosition, GetNetSyncPercent( ) );

		//This will eventually make this object disappear if we dont recieve messages for it
		m_timeOutTimer += Time::DELTA_SECONDS;
	}

	//Update sprite
	UpdateSprite( );

	//Update debug sprite
	if( Game::SHOW_COLLISION && !m_isHostObject )
	{
		UpdateNetDebugSprite( );
	}
}


//-------------------------------------------------------------------------------------------------
void GameObject::UpdateSprite( )
{
	m_sprite->SetID( GetSpriteID( ) );
	m_sprite->SetScale( GetSpriteScale( ) );
	m_sprite->SetPosition( m_position );
	m_sprite->SetRotation( -m_rotationDegrees );
	
	if( g_GameSystem->m_isDedicatedServer )
	{
		SetVisibility( true );
	}
	else if( m_isHostObject )
	{
		m_sprite->SetColor( Game::HOST_COLOR );
		SetVisibility( Game::s_showHost );
	}
}


//-------------------------------------------------------------------------------------------------
void GameObject::UpdateNetDebugSprite( )
{
	float spriteScale = ConvertToSpriteScale( GetPhysicsRadius( ), RADIUS_TO_SCALE );
	m_netDebugSprite->SetScale( spriteScale );
	m_netDebugSprite->SetPosition( m_position );
}


//-------------------------------------------------------------------------------------------------
bool GameObject::HostCheckCollision( GameObject * gameObject )
{
	if( OverlapDiscs( m_position, GetPhysicsRadius( ), gameObject->m_position, gameObject->GetPhysicsRadius( ) ) )
	{
		HostHandleCollision( gameObject );
		return true;
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
std::string GameObject::GetSpriteID( ) const
{
	return "error";
}


//-------------------------------------------------------------------------------------------------
float GameObject::GetElapsedGameTime( NetSender const & sender ) const
{
	return (float) ( ( sender.connection->GetRoundTripTime( ) - 1.0 / g_EngineSystem->GetTargetFPS( ) ) / 2.f );
}


//-------------------------------------------------------------------------------------------------
void GameObject::UpdateLocalData( float elapsedLocalGameTime )
{
	m_rotationDegrees = WrapDegrees0to360( m_rotationDegrees );
	m_position += m_velocity * elapsedLocalGameTime;
}


//-------------------------------------------------------------------------------------------------
void GameObject::UpdateNetData( float elapsedNetGameTime )
{
	m_netRotationDegrees = WrapDegrees0to360( m_netRotationDegrees );
	m_netPosition += m_netVelocity * elapsedNetGameTime;

	if( IsNotRecievingUpdates( ) )
	{
		m_position = m_netPosition;
		m_rotationDegrees = m_netRotationDegrees;
		m_velocity = m_netVelocity;
	}
}


//-------------------------------------------------------------------------------------------------
bool GameObject::IsClientOwner( ) const
{
	return g_GameSystem->GetSessionIndex( ) == m_playerIndex;
}


//-------------------------------------------------------------------------------------------------
void GameObject::HostOnDestroy( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void GameObject::ClientOnDestroy( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void GameObject::SetVisibility( bool isVisible )
{
	m_sprite->SetEnabled( isVisible );
}


//-------------------------------------------------------------------------------------------------
float GameObject::ConvertToSpriteScale( float radius, float desiredSize ) const
{
	return radius * desiredSize / g_SpriteRenderSystem->GetVirtualSize( );
}


//-------------------------------------------------------------------------------------------------
SoundID GameObject::GetRandomHitSound( ) const
{
	int randomSound = RandomInt( SOUND_FIRE_COUNT );
	return SOUND_HIT[randomSound];
}


//-------------------------------------------------------------------------------------------------
void GameObject::MessageRecieved( )
{
	m_timeOutTimer = 0.f;
}


//-------------------------------------------------------------------------------------------------
bool GameObject::IsNotRecievingUpdates( ) const
{
	return m_timeOutTimer > MAX_TIMEOUT_SECONDS;
}
