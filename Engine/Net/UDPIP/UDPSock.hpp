#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Utils/NetworkUtils.hpp"

//-------------------------------------------------------------------------------------------------
class UDPSock
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	SOCKET m_socket;
	sockaddr_in m_addr;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UDPSock( );

	void Bind( char const * addr, size_t port, size_t range );
	void Unbind( );

	bool IsConnected( ) const;
	char const * GetAddressString( ) const;
	sockaddr_in const & GetAddress( ) const;

protected:
	size_t Send( sockaddr_in addr, byte_t const * data, size_t dataSize ) const;
	size_t Recv( sockaddr_in * out_addr, byte_t * data, size_t maxSize /*max you can read into data*/ );
};