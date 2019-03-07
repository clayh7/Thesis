#pragma once

#include <memory>
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class TCPSocket
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static int const MAX_BACKLOG;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	friend class NetworkUtils;
	friend class RCSConnection;
	SOCKET m_socket;
	SocketAddressPtr m_address;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	~TCPSocket( );
	int Connect( SocketAddressPtr address );
	int Bind( SocketAddressPtr address );
	int Listen( int backlog = MAX_BACKLOG );
	TCPSocketPtr Accept( );
	int Send( void const * data, int length );
	int Receive( void * buffer, int length );
	void SetBlocking( bool isBlocking );

	std::string GetAddressString( ) const;

private:
	TCPSocket( SOCKET inSocket );
	void SetAddress( SocketAddress const & address );
};
typedef std::shared_ptr<TCPSocket> TCPSocketPtr;