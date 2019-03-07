#include "Engine/Net/NetworkSystem.hpp"

#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC size_t NetworkSystem::GAME_PORT = 4334;
STATIC char const * NetworkSystem::NETWORK_STARTUP = " NetworkStartup";
STATIC char const * NetworkSystem::NETWORK_SHUTDOWN = "NetworkShutdown";
STATIC char const * NetworkSystem::NETWORK_UPDATE_EVENT = "NetworkUpdateEvent";


//-------------------------------------------------------------------------------------------------
STATIC bool NetworkSystem::Startup( )
{
	WSADATA wsa_data;
	int error = WSAStartup( MAKEWORD( 2, 2 ), &wsa_data ); //version 2.2
	if( error == SOCKET_ERROR )
	{
		NetworkUtils::ReportError( );
	}

	EventSystem::RegisterEvent( ENGINE_UPDATE_EVENT, &NetworkSystem::OnUpdate );
	EventSystem::TriggerEvent( NETWORK_STARTUP );

	return true;
}


//-------------------------------------------------------------------------------------------------
STATIC void NetworkSystem::Shutdown( )
{
	EventSystem::TriggerEvent( NETWORK_SHUTDOWN );

	int error = WSACleanup( );
	if( error == SOCKET_ERROR )
	{
		NetworkUtils::ReportError( );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void NetworkSystem::OnUpdate( NamedProperties & )
{
	EventSystem::TriggerEvent( NetworkSystem::NETWORK_UPDATE_EVENT );
}