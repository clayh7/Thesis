#pragma once

#include <vector>
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class NamedProperties;
class RCSConnection;
class Command;


//-------------------------------------------------------------------------------------------------
void RCSHost( Command const & );
void RCSJoin( Command const & );
void RCSLeave( Command const & );
void RCSInfo( Command const & );
void RCSSend( Command const & );


//-------------------------------------------------------------------------------------------------
enum eRCSState
{
	eRCSState_HOST,
	eRCSState_CLIENT,
	eRCSState_DISCONNECTED,
};


//-------------------------------------------------------------------------------------------------
class RemoteCommandServer
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static RemoteCommandServer * s_remoteCommandServer;
	static int const DEFAULT_WAIT;
	static size_t RCS_PORT;

public:
	static char const * RCS_MESSAGE_EVENT;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	eRCSState m_state;
	TCPSocketPtr m_listener;
	std::vector<RCSConnection*> m_connections;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void Startup( );
	static void Shutdown( );
	static bool Host( uint32_t port = RCS_PORT );
	static bool Join( char const * host, uint32_t port = RCS_PORT );
	static bool Send( eRCSMessageType const & type, std::string const & message );
	static bool Leave( );
	static RemoteCommandServer const * Get( ){ return s_remoteCommandServer; }

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
private:
	RemoteCommandServer( ); 
	~RemoteCommandServer( );

	void OnUpdate( NamedProperties & params );
	void OnMessage( NamedProperties & params );
	void StartTCPListener( SocketAddressPtr address );
	void CreateRCSConnection( SocketAddressPtr address );
	void CreateRCSConnection( TCPSocketPtr tcpSocket );
	void SendRCSMessage( eRCSMessageType const & type, std::string const & message );
	void Disconnect( );

	void AcceptNewConnections( );
	void UpdateConnections( );
	void FDArrayFill( std::vector<WSAPOLLFD> & fdArray );
	void FDArrayProcess( std::vector<WSAPOLLFD> & fdArray );
	void HandleDisconnect( RCSConnection * conn );

public:
	bool IsConnected( ) const;
	bool IsHost( ) const;
	bool IsClient( ) const;
	std::string GetListenAddress( ) const;
	std::vector<std::string> GetConnectionAddresses( ) const;
};