#pragma once


//-------------------------------------------------------------------------------------------------
class NamedProperties;


//-------------------------------------------------------------------------------------------------
class NetworkSystem
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static size_t GAME_PORT;
	static char const * NETWORK_STARTUP;
	static char const * NETWORK_SHUTDOWN;
	static char const * NETWORK_UPDATE_EVENT;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static bool Startup( );
	static void Shutdown( );

private:
	static void OnUpdate( NamedProperties & );
};