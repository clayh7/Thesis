#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class SocketAddress
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	friend class TCPSocket;
	friend class UDPSocket;
	sockaddr m_sockAddr;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	SocketAddress( uint32_t address, uint16_t port );
	SocketAddress( sockaddr const & sockAddr );

	size_t GetSize( ) const;
	char const * GetAddress( );

private:
	sockaddr_in * GetAsSockAddrIn( );
};