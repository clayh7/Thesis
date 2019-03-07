#include "Engine/Net/Sockets/SocketAddress.hpp"


//-------------------------------------------------------------------------------------------------
//This is for IPv4, #TODO: make a IPv6 constructor
SocketAddress::SocketAddress( uint32_t address, uint16_t port )
{
	sockaddr_in * addr = GetAsSockAddrIn( );
	addr->sin_family = AF_INET; //IPv4
	addr->sin_addr.S_un.S_addr = htonl( address );
	addr->sin_port = htons( port );
}


//-------------------------------------------------------------------------------------------------
SocketAddress::SocketAddress( sockaddr const & sockAddr )
{
	memcpy( &m_sockAddr, &sockAddr, sizeof( sockaddr ) );
}


//-------------------------------------------------------------------------------------------------
size_t SocketAddress::GetSize( ) const
{
	return sizeof( sockaddr );
}


//-------------------------------------------------------------------------------------------------
char const * SocketAddress::GetAddress( )
{
	int const tempBufferSize = 256;

	// buffer is static - so will not go out of scope, but that means this is not thread safe.
	static char buffer[tempBufferSize];
	char hostname[tempBufferSize];
	sockaddr_in * addr = GetAsSockAddrIn( );

	// inet_ntop converts an address type to a human readable string
	inet_ntop( addr->sin_family, &(addr->sin_addr), hostname, tempBufferSize );

	// Combine the host with the port  
	sprintf_s( buffer, tempBufferSize, "%s:%u", hostname, ntohs( addr->sin_port ) );

	return buffer;
}


//-------------------------------------------------------------------------------------------------
sockaddr_in * SocketAddress::GetAsSockAddrIn( )
{
	return reinterpret_cast<sockaddr_in*>( &m_sockAddr );
}