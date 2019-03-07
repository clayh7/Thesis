#pragma once

#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class SocketAddress;


//-------------------------------------------------------------------------------------------------
class UDPSocket
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	friend class NetworkUtils;
	SOCKET m_socket;
	SocketAddressPtr m_address;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	~UDPSocket( );
	int Bind( SocketAddressPtr address );
	int SendTo( void const * data, int length, SocketAddressPtr toAddress );
	int ReceiveFrom( void * buffer, int length, SocketAddressPtr out_fromAddress );
	void SetBlocking( bool isBlocking );

private:
	UDPSocket( SOCKET inSocket );
};
typedef std::shared_ptr<UDPSocket> UDPSocketPtr;