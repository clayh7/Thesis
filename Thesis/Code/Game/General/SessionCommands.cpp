#include "Game/General/SessionCommands.hpp"

#include "Engine/DebugSystem/Command.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/Net/NetworkSystem.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
void RegisterConsoleCommands( )
{
	g_ConsoleSystem->RegisterCommand( "session_start", SessionStartCommand, ": [port] [range] Start a net session on (port), look up to (range) times. default = 4334 | 8" );
	g_ConsoleSystem->RegisterCommand( "session_stop", SessionStopCommand, ": Stop current net session." );
	g_ConsoleSystem->RegisterCommand( "session_host", SessionHostCommand, ": [guid] Start a host connection. default = Host" );
	g_ConsoleSystem->RegisterCommand( "session_join", SessionJoinCommand, " [address] [guid] : Join host with this guid." );
	g_ConsoleSystem->RegisterCommand( "session_leave", SessionLeaveCommand, " : Destroy your connection and all other connections." );
	g_ConsoleSystem->RegisterCommand( "session_create_connection", SessionCreateConnectionCommand, " [index] [name] [address] : Set address as the (index)th connection." );

	g_ConsoleSystem->RegisterCommand( "session_drop_rate", SessionDropRateCommand, " [rate] : Chance to drop incoming packet (0.0-1.0)." );
	g_ConsoleSystem->RegisterCommand( "session_latency", SessionLatencyCommand, " [min] [max] : Delay before processing incoming packet (miliseconds)." );
	g_ConsoleSystem->RegisterCommand( "session_toggle_timeouts", SessionToggleTimeoutsCommand, " : Connections are automatically destroyed after a long time of no traffic." );

	g_ConsoleSystem->RegisterCommand( "session_debug", SessionDebug, " : Debugs Connection traffic." );
}


//-------------------------------------------------------------------------------------------------
void SessionStartCommand( Command const & command )
{
	size_t port = command.GetArg( 0, NetworkSystem::GAME_PORT );
	size_t range = command.GetArg( 1, 8U );
	Game::s_netSession->Start( port, range );
}


//-------------------------------------------------------------------------------------------------
void SessionStopCommand( Command const & )
{
	Game::s_netSession->Stop( );
}


//-------------------------------------------------------------------------------------------------
void SessionHostCommand( Command const & command )
{
	std::string username = NetworkUtils::GetLocalHostName( );
	username = command.GetArg( 0, username );
	std::string password = command.GetArg( 1, "" );
	g_GameSystem->AttemptCreateHost( username, password );
}


//-------------------------------------------------------------------------------------------------
void SessionJoinCommand( Command const & command )
{
	if( command.HasArg( 0 ) )
	{
		std::string addressString = command.GetArg( 0, "error" );
		std::string username = NetworkUtils::GetLocalHostName( );
		username = command.GetArg( 1, username );
		std::string password = command.GetArg( 2, "" );
		g_GameSystem->AttemptCreateClient( addressString, username, password );
	}
	else
	{
		g_ConsoleSystem->AddLog( "Not enough arguments", Console::BAD );
	}
}


//-------------------------------------------------------------------------------------------------
void SessionLeaveCommand( Command const & )
{
	Game::s_netSession->Leave( );
}


//-------------------------------------------------------------------------------------------------
void SessionCreateConnectionCommand( Command const & command )
{
	if( command.HasArg( 2 ) )
	{
		byte_t id = command.GetArg( 0, (byte_t) 0U );
		std::string guid = command.GetArg( 1, "unknown" );
		std::string addressString = command.GetArg( 2, "error" );
		sockaddr_in address;
		if( SockAddrFromString( &address, addressString.c_str( ) ) )
		{
			//#TODO: Allow specification of username when manually making a connection... or remove this function entirely
			NetConnection * newConnection = Game::s_netSession->CreateConnection( id, address, guid, NetworkUtils::GetLocalHostName( ) );
			if( newConnection )
			{
				Game::s_netSession->Connect( newConnection );
			}
		}
		else
		{
			g_ConsoleSystem->AddLog( "Invalid Address", Console::BAD );
		}

		return;
	}

	g_ConsoleSystem->AddLog( "Not enough arguments", Console::BAD );
}


//-------------------------------------------------------------------------------------------------
void SessionDropRateCommand( Command const & command )
{
	float dropRate = command.GetArg( 0, 0.f );
	dropRate = Clamp( dropRate, 0.f, 1.f );

	Game::s_netSession->SetDropRate( dropRate );
	g_ConsoleSystem->AddLog( Stringf( "Set drop rate: %.2f", dropRate ), Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
void SessionLatencyCommand( Command const & command )
{
	double minimum = command.GetArg( 0, 0 );
	double maximum = minimum;
	if( command.HasArg( 1 ) )
	{
		maximum = command.GetArg( 1, maximum );
		if( maximum < minimum )
		{
			double temp = maximum;
			maximum = minimum;
			minimum = temp;
		}
	}

	Game::s_netSession->SetLatency( Range<double>( minimum / 1000.0, maximum / 1000.0 ) );
	g_ConsoleSystem->AddLog( Stringf( "Set latency: %d - %d", (int) minimum, (int) maximum ), Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
void SessionToggleTimeoutsCommand( Command const & )
{
	bool timeouts = Game::s_netSession->ToggleTimeouts( );
	g_ConsoleSystem->AddLog( Stringf( "Connection timeouts: %s", timeouts ? "enabled" : "disabled" ), Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
void SessionDebug( Command const & )
{
	g_GameSystem->m_showDebug = !g_GameSystem->m_showDebug;
}