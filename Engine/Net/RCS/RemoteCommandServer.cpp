#include "Engine/Net/RCS/RemoteCommandServer.hpp"

#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/Net/NetworkSystem.hpp"
#include "Engine/Net/RCS/RCSConnection.hpp"
#include "Engine/Net/Sockets/TCPSocket.hpp"
#include "Engine/Net/Sockets/SocketAddress.hpp"


//-------------------------------------------------------------------------------------------------
STATIC RemoteCommandServer * RemoteCommandServer::s_remoteCommandServer = nullptr;
STATIC const int RemoteCommandServer::DEFAULT_WAIT = 0;
STATIC size_t RemoteCommandServer::RCS_PORT = 4325;

STATIC char const * RemoteCommandServer::RCS_MESSAGE_EVENT = "RCSMessageEvent";


//-------------------------------------------------------------------------------------------------
void RCSHost( Command const & )
{
	RemoteCommandServer const * rcs = RemoteCommandServer::Get( );
	if( rcs->IsHost( ) )
	{
		g_ConsoleSystem->AddLog( "Already hosting.", Console::BAD );
		return;
	}

	if( rcs->IsClient( ) )
	{
		g_ConsoleSystem->AddLog( "Clients can not also host.", Console::BAD );
		return;
	}

	if( RemoteCommandServer::Host( ) )
	{
		std::string success = Stringf( "Host successful. Listening on %s", rcs->GetListenAddress( ).c_str( ) );
		g_ConsoleSystem->AddLog( success, Console::GOOD );
	}
	else
	{
		g_ConsoleSystem->AddLog( "Failed to host.", Console::BAD );
	}
}


//-------------------------------------------------------------------------------------------------
void RCSJoin( Command const & command )
{
	RemoteCommandServer const * rcs = RemoteCommandServer::Get( );
	if( rcs->IsConnected( ) )
	{
		g_ConsoleSystem->AddLog( "Already connected to host.", Console::BAD );
		return;
	}

	//Parse host address string
	std::string address = "";
	if( !command.HasArg( 0 ) )
	{
		address = NetworkUtils::GetLocalHostName( );
	}
	else
	{
		address = command.GetArg( 0, "error" );
	}

	if( RemoteCommandServer::Join( address.c_str( ) ) )
	{
		std::string success = Stringf( "Joined server: %s", address.c_str( ) );
		g_ConsoleSystem->AddLog( success, Console::GOOD );
	}
	else
	{
		g_ConsoleSystem->AddLog( "Failed to join server.", Console::BAD );
	}
}


//-------------------------------------------------------------------------------------------------
void RCSLeave( Command const & )
{
	RemoteCommandServer const * rcs = RemoteCommandServer::Get( );
	if( !rcs->IsConnected( ) )
	{
		g_ConsoleSystem->AddLog( "Nothing to leave.", Console::BAD );
		return;
	}

	if( rcs->IsHost( ) )
	{
		RemoteCommandServer::Leave( );
		g_ConsoleSystem->AddLog( "Stopped hosting.", Console::GOOD );
	}

	if( rcs->IsClient( ) )
	{
		RemoteCommandServer::Leave( );
		g_ConsoleSystem->AddLog( "Disconnected from host.", Console::GOOD );
	}
}


//-------------------------------------------------------------------------------------------------
void RCSInfo( Command const & )
{
	RemoteCommandServer const * rcs = RemoteCommandServer::Get( );
	if( rcs->IsHost( ) )
	{
		g_ConsoleSystem->AddLog( "Status: Host", Console::INFO );
		std::string hostString = Stringf( "Host: %s", rcs->GetListenAddress( ).c_str( ) );
		g_ConsoleSystem->AddLog( hostString , Console::GOOD );

		std::vector<std::string> connectionAddressList = rcs->GetConnectionAddresses( );
		g_ConsoleSystem->AddLog( Stringf( "Connections: %u", connectionAddressList.size( ) ), Console::GOOD );
		for( size_t clientIndex = 0; clientIndex < connectionAddressList.size(); ++clientIndex )
		{
			std::string clientString = Stringf( "[%u] %s", clientIndex, connectionAddressList[clientIndex].c_str( ) );
			g_ConsoleSystem->AddLog( clientString, Console::GOOD );
		}
	}
	else if( rcs->IsClient( ) )
	{
		g_ConsoleSystem->AddLog( "Status: Client", Console::INFO );
		std::vector<std::string> connectionAddressList = rcs->GetConnectionAddresses( );
		std::string clientString = Stringf( "Client: %s", connectionAddressList[0].c_str( ) );
		g_ConsoleSystem->AddLog( clientString, Console::GOOD );
	}
	else
	{
		g_ConsoleSystem->AddLog( "Status: Disconnected", Console::INFO );
	}
}


//-------------------------------------------------------------------------------------------------
void RCSSend( Command const & command )
{
	RemoteCommandServer const * rcs = RemoteCommandServer::Get( );
	if( !rcs->IsConnected( ) )
	{
		g_ConsoleSystem->AddLog( "Not connected.", Console::BAD );
		return;
	}

	RemoteCommandServer::Send( eRCSMessageType_COMMAND, command.GetRemainingString( 0 ) );
}


//-------------------------------------------------------------------------------------------------
STATIC void RemoteCommandServer::Startup( )
{
	if( s_remoteCommandServer == nullptr )
	{
		s_remoteCommandServer = new RemoteCommandServer( );
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::Shutdown( )
{
	if( s_remoteCommandServer != nullptr )
	{
		delete s_remoteCommandServer;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Host( uint32_t port /*= RCS_PORT*/)
{
	if( !s_remoteCommandServer->IsConnected( ) )
	{
		char const * host = NetworkUtils::GetLocalHostName( );
		SocketAddressPtr address = NetworkUtils::CreateIPv4FromString( host, port );
		s_remoteCommandServer->StartTCPListener( address );
		return s_remoteCommandServer->IsHost( );
	}

	//Already host or client
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Join( char const * host, uint32_t port /*= RCS_PORT*/ )
{
	if( !s_remoteCommandServer->IsConnected( ) )
	{
		if( s_remoteCommandServer->m_connections.size( ) > 0 )
		{
			return false;
		}
		SocketAddressPtr address = NetworkUtils::CreateIPv4FromString( host, port );
		s_remoteCommandServer->CreateRCSConnection( address );
		return s_remoteCommandServer->IsClient( );
	}

	//Already host or client
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Send( eRCSMessageType const & type, std::string const & message )
{
	if( s_remoteCommandServer->IsConnected( ) )
	{
		s_remoteCommandServer->SendRCSMessage( type, message );
	}

	//Not connected
	return false;
}


//-------------------------------------------------------------------------------------------------
STATIC bool RemoteCommandServer::Leave( )
{
	if( s_remoteCommandServer->IsConnected( ) )
	{
		s_remoteCommandServer->Disconnect( );
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
RemoteCommandServer::RemoteCommandServer( )
	: m_state( eRCSState_DISCONNECTED )
{
	EventSystem::RegisterEvent( NetworkSystem::NETWORK_UPDATE_EVENT, this, &RemoteCommandServer::OnUpdate );
	EventSystem::RegisterEvent( RCS_MESSAGE_EVENT, this, &RemoteCommandServer::OnMessage );

	Console::RegisterCommand( "rcs_host", RCSHost, ": Host a local server." );
	Console::RegisterCommand( "rcs_join", RCSJoin, " [host address] : Join server." );
	Console::RegisterCommand( "rcs_leave", RCSLeave, ": Leave current server." );
	Console::RegisterCommand( "rcs_info", RCSInfo, ": Prints current network state." );
	Console::RegisterCommand( "rcs_send", RCSSend, " [cmd] : Sends command to all connections." );
}


//-------------------------------------------------------------------------------------------------
RemoteCommandServer::~RemoteCommandServer( )
{
	for( RCSConnection * conn : m_connections )
	{
		delete conn;
		conn = nullptr;
	}
	m_connections.clear( );
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::OnUpdate( NamedProperties & )
{
	if( !IsConnected( ) )
	{
		return;
	}

	AcceptNewConnections( );
	UpdateConnections( );
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::OnMessage( NamedProperties & params )
{
	eRCSMessageType messageType;
	params.Get( "MessageType", messageType );
	std::string message;
	params.Get( "Message", message );

	if( messageType == eRCSMessageType_COMMAND )
	{
		g_ConsoleSystem->RunCommand( message, true );
	}
	else if( messageType == eRCSMessageType_ECHO )
	{
		g_ConsoleSystem->AddLog( Stringf( "REMOTE: %s", message.c_str( ) ), Console::REMOTE, true );
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::StartTCPListener( SocketAddressPtr address )
{
	m_listener = NetworkUtils::CreateTCPSocket( eSocketAddressFamily_IPv4 );
	if( m_listener == nullptr )
	{
		return;
	}
	int error = m_listener->Bind( address );
	if( error != NO_ERROR )
	{
		return;
	}
	error = m_listener->Listen( );
	if( error == NO_ERROR )
	{
		m_state = eRCSState_HOST;
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::CreateRCSConnection( SocketAddressPtr address )
{
	TCPSocketPtr tcpSocket = NetworkUtils::CreateTCPSocket( eSocketAddressFamily_IPv4 );
	if( tcpSocket != nullptr )
	{
		//Must block before connect
		tcpSocket->SetBlocking( true );
		int error = tcpSocket->Connect( address );
		if( error == NO_ERROR )
		{
			CreateRCSConnection( tcpSocket );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::CreateRCSConnection( TCPSocketPtr tcpSocket )
{
	tcpSocket->SetBlocking( false );
	RCSConnection * connection = new RCSConnection( tcpSocket );
	m_connections.push_back( connection );

	if( !IsConnected( ) )
	{
		m_state = eRCSState_CLIENT;
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::SendRCSMessage( eRCSMessageType const & type, std::string const & message )
{
	for( RCSConnection const * conn : m_connections )
	{
		conn->Send( type, message.c_str( ) );
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::Disconnect( )
{
	if( IsHost( ) )
	{
		m_listener = nullptr;
		for( RCSConnection * conn : m_connections )
		{
			delete conn;
			conn = nullptr;
		}
		m_connections.clear( );
		m_state = eRCSState_DISCONNECTED;
	}

	if( IsClient( ) )
	{
		ASSERT_RECOVERABLE( m_connections.size( ) == 1, "Incorrect number of client connections." )
			delete m_connections[0];
		m_connections[0] = nullptr;
		m_connections.clear( );
		m_state = eRCSState_DISCONNECTED;
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::AcceptNewConnections( )
{
	if( !IsHost( ) )
	{
		return;
	}

	TCPSocketPtr tcpSocket = m_listener->Accept( );
	if( tcpSocket != nullptr )
	{
		CreateRCSConnection( tcpSocket );
		g_ConsoleSystem->AddLog( "Client connected: " + tcpSocket->GetAddressString( ), Console::REMOTE );
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::UpdateConnections( )
{
	std::vector<WSAPOLLFD> fdArray;
	FDArrayFill( fdArray );
	if( fdArray.size( ) <= 0 )
	{
		return;
	}

	uint32_t result = WSAPoll( &fdArray[0], fdArray.size( ), DEFAULT_WAIT );
	if( result <= 0 )
	{
		return;
	}

	FDArrayProcess( fdArray );
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::FDArrayFill( std::vector<WSAPOLLFD> & fdArray )
{
	for( size_t connIndex = 0; connIndex < m_connections.size( ); ++connIndex )
	{
		RCSConnection * conn = m_connections[connIndex];
		WSAPOLLFD poll;
		poll.fd = conn->GetSocket( );
		poll.events = POLLRDNORM | POLLWRNORM;
		fdArray.push_back( poll );
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::FDArrayProcess( std::vector<WSAPOLLFD> & fdArray )
{
	for( size_t fdIndex = 0; fdIndex < fdArray.size( ); )
	{
		//Connection can be read from
		if( fdArray[fdIndex].revents & POLLRDNORM )
		{
			m_connections[fdIndex]->Receive( );
		}
		
		//Connection can be sent to
		if( fdArray[fdIndex].revents & POLLWRNORM )
		{
			//Nothing
		}

		//Connection left
		if( fdArray[fdIndex].revents & POLLHUP )
		{
			HandleDisconnect( m_connections[fdIndex] );

			//Remove from fdArray
			std::vector<WSAPOLLFD>::iterator fdIter = fdArray.begin( ) + fdIndex;
			fdArray.erase( fdIter );

			//Remove from 
			std::vector<RCSConnection*>::iterator connIter = m_connections.begin( ) + fdIndex;
			m_connections.erase( connIter );
		}
		else
		{
			++fdIndex;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void RemoteCommandServer::HandleDisconnect( RCSConnection * conn )
{
	if( IsHost( ) )
	{
		g_ConsoleSystem->AddLog( Stringf( "Client left: %s", conn->GetAddress( ).c_str( ) ), Console::REMOTE );
	}

	if( IsClient( ) )
	{
		g_ConsoleSystem->AddLog( Stringf( "Host disconnected." ), Console::REMOTE );
		m_state = eRCSState_DISCONNECTED;
	}

	delete conn;
}


//-------------------------------------------------------------------------------------------------
bool RemoteCommandServer::IsConnected( ) const
{
	return m_state != eRCSState_DISCONNECTED;
}


//-------------------------------------------------------------------------------------------------
bool RemoteCommandServer::IsHost( ) const
{
	return m_state == eRCSState_HOST;
}


//-------------------------------------------------------------------------------------------------
bool RemoteCommandServer::IsClient( ) const
{
	return m_state == eRCSState_CLIENT;
}


//-------------------------------------------------------------------------------------------------
std::string RemoteCommandServer::GetListenAddress( ) const
{
	return m_listener->GetAddressString( );
}


//-------------------------------------------------------------------------------------------------
std::vector<std::string> RemoteCommandServer::GetConnectionAddresses( ) const
{
	std::vector<std::string> connectionAddressList;

	for( RCSConnection * conn : m_connections )
	{
		connectionAddressList.push_back( conn->GetAddress( ) );
	}

	return connectionAddressList;
}
