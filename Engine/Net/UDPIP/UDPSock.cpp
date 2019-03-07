#include "Engine/Net/UDPIP/UDPSock.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
UDPSock::UDPSock( )
	: m_socket( INVALID_SOCKET )
	, m_addr( )
{

}


//-------------------------------------------------------------------------------------------------
// To be used as a reference
SOCKET CreateUDPSocektExample( char const * addr,
	char const * service, // who we're trying to connect to
	sockaddr_in * out_addr ) // address we actually connected to.
{
	// First, try to get network addresses for this
	//#TODO: Change this to AF_INET6
	addrinfo * info_list = AllocAddressesForHost( addr, // an address for this machine
		service, // service, which for TCP/IP is the port as a string (ex: "80")
		AF_INET, // We're doing IPv4 in class
		SOCK_DGRAM, // UDP for now
		AI_PASSIVE );  // And something we can bind (and therefore listen on)

	if( info_list == nullptr )
	{
		// no addresses match - FAIL
		return false;
	}

	// Alright, try to create a SOCKET from this addr info
	SOCKET my_socket = INVALID_SOCKET;
	addrinfo * iter = info_list;
	while( ( iter != nullptr ) && ( my_socket == INVALID_SOCKET ) )
	{
		// First, create a socket for this address.
		// family, socktype, and protocol are provided by the addrinfo
		// if you wanted to be manual, for an TCP/IPv4 socket you'd use
		// AF_INET, SOCK_DGRAM, IPPROTO_UDP
		my_socket = socket( iter->ai_family, iter->ai_socktype, iter->ai_protocol );
		if( my_socket != INVALID_SOCKET )
		{
			// Okay, we were able to create it,
			// Now try to bind it (associates the address (ex: 192.168.1.52:4325) to this 
			// socket so it will receive information for it.
			int result = bind( my_socket, iter->ai_addr, (int) ( iter->ai_addrlen ) );
			if( SOCKET_ERROR != result )
			{
				// Set it to non-block - since we'll be working with this on our main thread
				SetBlocking( my_socket, false );

				// Save off the address if available.
				ASSERT_RECOVERABLE( iter->ai_addrlen == sizeof( sockaddr_in ), "Invalid UDP address length." );
				if( nullptr != out_addr )
				{
					memcpy( out_addr, iter->ai_addr, iter->ai_addrlen );
				}
			}
			else
			{
				// Cleanup on Fail.
				closesocket( my_socket );
				my_socket = INVALID_SOCKET;
			}
		}
		iter = iter->ai_next;
	}

	// If we allocted, we must free eventually
	FreeAddresses( info_list );

	// Return the socket we created.
	return my_socket;
}


//-------------------------------------------------------------------------------------------------
// Binding a TCP Socket for Listening Purposes
SOCKET CreateUDPSocekt( char const * addr, size_t port, sockaddr_in * out_addr, size_t range )
{
	//stick first port in as a string
	addrinfo * info_list = AllocAddressesForHost( addr, Stringf( "%u", port ).c_str( ), AF_INET, SOCK_DGRAM, AI_PASSIVE );

	if( info_list == nullptr )
	{
		return false;
	}

	SOCKET my_socket = INVALID_SOCKET;
	addrinfo * iter = info_list;
	while( ( iter != nullptr ) && ( my_socket == INVALID_SOCKET ) )
	{
		my_socket = socket( iter->ai_family, iter->ai_socktype, iter->ai_protocol );
		if( my_socket != INVALID_SOCKET )
		{
			int result = SOCKET_ERROR;
			//Test all ports, then continue to the next available family
			for( size_t testPort = port; testPort < port + range; ++testPort )
			{
				sockaddr_in * testAddress = (sockaddr_in*) iter->ai_addr;
				testAddress->sin_port = htons( (uint16_t) testPort );
				result = bind( my_socket, (sockaddr*) testAddress, (int) ( iter->ai_addrlen ) );
				if( SOCKET_ERROR != result )
				{
					// Set it to non-block - since we'll be working with this on our main thread
					SetBlocking( my_socket, false );

					// Save off the address if available.
					ASSERT_RECOVERABLE( iter->ai_addrlen == sizeof( sockaddr_in ), "Invalid UDP address length." );
					if( nullptr != out_addr )
					{
						memcpy( out_addr, iter->ai_addr, iter->ai_addrlen );
					}
					break;
				}
			}

			//If no connection, clean up socket and continue to the next one
			if( SOCKET_ERROR == result )
			{
				// Cleanup on Fail.
				closesocket( my_socket );
				my_socket = INVALID_SOCKET;
			}
		}
		iter = iter->ai_next;
	}

	// If we allocted, we must free eventually
	FreeAddresses( info_list );

	// Return the socket we created.
	return my_socket;
}


//-------------------------------------------------------------------------------------------------
void UDPSock::Bind( char const * addr, size_t port, size_t range )
{
	m_socket = CreateUDPSocekt( addr, port, &m_addr, range );
}


//-------------------------------------------------------------------------------------------------
void UDPSock::Unbind( )
{
	closesocket( m_socket );
	m_socket = INVALID_SOCKET;
}


//-------------------------------------------------------------------------------------------------
bool UDPSock::IsConnected( ) const
{
	return m_socket != INVALID_SOCKET;
}


//-------------------------------------------------------------------------------------------------
char const * UDPSock::GetAddressString( ) const
{
	return StringFromSockAddr( &m_addr );
	//return nullptr;
	//#TODO: This function needs to use the updated modern style from the book I'm reading
	//It's been a while so I don't remember why I needed to change this
}


//-------------------------------------------------------------------------------------------------
sockaddr_in const & UDPSock::GetAddress( ) const
{
	return m_addr;
}


//-------------------------------------------------------------------------------------------------
size_t SocketSendTo( SOCKET mySocket,
	sockaddr_in & toAddr,
	void const * data,
	size_t const dataSize )
{
	if( mySocket != INVALID_SOCKET )
	{
		// send will return the amount of data actually sent.
		// It SHOULD match, or be an error.  
		int size = ::sendto( mySocket,
			(char const*) data,      // payload
			(int) dataSize,         // payload size
			0,                      // flags - unused
			(sockaddr const*) &toAddr, // who we're sending to
			sizeof( sockaddr_in ) );  // size of that structure

		if( size > 0 )
		{
			return size;
		}
	}

	// Not particularly interested in errors - you can 
	// check this though if you want to see if something
	// has happened to your socket.
	return 0;
}


//-------------------------------------------------------------------------------------------------
size_t UDPSock::Send( sockaddr_in addr, byte_t const * data, size_t dataSize ) const
{
	return SocketSendTo( m_socket, addr, data, dataSize );
}


//-------------------------------------------------------------------------------------------------
size_t SocketReceiveFrom( sockaddr_in * out_fromAddr,
	SOCKET mySocket,
	void * data,
	size_t const maxSize )
{
	if( mySocket != INVALID_SOCKET )
	{
		// recv will return amount of data read, should always be <= buffer_size
		// Also, if you send, say, 3 KB with send, recv may actually
		// end up returning multiple times (say, 1KB, 512B, and 1.5KB) because 
		// the message got broken up - so be sure you application watches for it

		sockaddr_storage addr;
		int addrlen = sizeof( addr );

		int size = ::recvfrom( mySocket,
			(char*) data,    // what we're reading into
			maxSize,      // max data we can read
			0,                // optional flags (see docs if you're curious)
			(sockaddr*) &addr, // Who sent the message
			&addrlen );       // length of their address

		if( size > 0 )
		{
			// We're only doing IPv4 - if we got a non-IPv4 address
			// assume it's garbage
			ASSERT_RECOVERABLE( addrlen == sizeof( sockaddr_in ), "Incorrect address length." );

			memcpy( out_fromAddr, &addr, addrlen );
			return size;
		}
	}

	// Again, I don't particularly care about the 
	// error code for now.  It may tell us
	// the guy we're sending to is bad, but we can't really
	// do anything with that yet. 
	return 0U;
}


//-------------------------------------------------------------------------------------------------
size_t UDPSock::Recv( sockaddr_in * out_addr, byte_t * data, size_t maxSize /*max you can read into data*/ )
{
	return SocketReceiveFrom( out_addr, m_socket, data, maxSize );
}