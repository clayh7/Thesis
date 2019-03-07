#include "Game/GameObjects/NetGameObject.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Game/General/Game.hpp"
#include "Game/GameObjects/GameObject.hpp"


//-------------------------------------------------------------------------------------------------
STATIC size_t NetGameObject::s_nextNetID = 1;


//-------------------------------------------------------------------------------------------------
NetGameObject::NetGameObject( GameObject * linkedGameObject )
	: m_netID( s_nextNetID++ )
	, m_localObject( linkedGameObject )
{
	if( !g_GameSystem->IsHost( ) )
	{
		ERROR_AND_DIE( "Can not create NetGameObjects if you're not the host." );
	}
}


//-------------------------------------------------------------------------------------------------
NetGameObject::NetGameObject( size_t netID, GameObject * linkedGameObject )
	: m_netID( netID )
	, m_localObject( linkedGameObject )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
NetGameObject::~NetGameObject( )
{
	delete m_localObject;
	m_localObject = nullptr;
}


//-------------------------------------------------------------------------------------------------
void NetGameObject::WriteToMessage( NetMessage * out_message )
{
	Game::WriteUncompressedUint32( out_message, m_netID );

	//Only if we're creating a new object do we need to know its type.
	//Otherwise that information id bound to the ID of the objects from then on.
	eNetGameMessageType messageType = (eNetGameMessageType) out_message->GetNetMessageType( );
	if( messageType == eNetGameMessageType_NET_OBJECT_CREATE )
	{
		out_message->Write<eNetGameObjectType>( m_localObject->m_type );
	}

	m_localObject->WriteToMessage( out_message );
}


//-------------------------------------------------------------------------------------------------
size_t NetGameObject::GetID( ) const
{
	return m_netID;
}


//-------------------------------------------------------------------------------------------------
GameObject * NetGameObject::GetLocalObject( ) const
{
	return m_localObject;
}


//-------------------------------------------------------------------------------------------------
eNetGameObjectType NetGameObject::GetType( ) const
{
	return m_localObject->m_type;
}