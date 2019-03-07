#include "Engine/Utils/NetworkUtils.hpp"

#include <string>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Net/Sockets/SocketAddress.hpp"
#include "Engine/Net/Sockets/TCPSocket.hpp"
#include "Engine/Net/Sockets/UDPSocket.hpp"


//-------------------------------------------------------------------------------------------------
STATIC int const NetworkUtils::WOULD_BLOCK_ERROR = 10035;


//-------------------------------------------------------------------------------------------------
STATIC SocketAddressPtr NetworkUtils::CreateIPv4FromString( std::string const & addressString )
{
	size_t portPosition = addressString.find_last_of( ':' );
	std::string host, service;
	if( portPosition != std::string::npos )
	{
		host = addressString.substr( 0, portPosition );
		service = addressString.substr( portPosition + 1 );
	}
	else
	{
		host = addressString;
		//use default port...
		service = "0";
	}
	return NetworkUtils::CreateIPv4FromString( host, service );
}


//-------------------------------------------------------------------------------------------------
STATIC SocketAddressPtr NetworkUtils::CreateIPv4FromString( std::string const & host, uint32_t port )
{
	std::string service = Stringf( "%u", port );
	return NetworkUtils::CreateIPv4FromString( host, service );
}


//-------------------------------------------------------------------------------------------------
// port and service mean the same thing
STATIC SocketAddressPtr NetworkUtils::CreateIPv4FromString( std::string const & host, std::string const & service )
{
	addrinfo hint;
	//Clear it out
	memset( &hint, 0, sizeof( hint ) );
	//IPv4
	hint.ai_family = AF_INET;

	addrinfo * result;
	int error = getaddrinfo( host.c_str( ), service.c_str( ), &hint, &result );
	addrinfo * start = result;
	if( error == SOCKET_ERROR && result != nullptr )
	{
		freeaddrinfo( start );
		return nullptr;
	}

	if( result == nullptr )
	{
		return nullptr;
	}

	//Keep searching for a non-nullptr entry
	while( result->ai_addr == nullptr && result->ai_next != nullptr )
	{
		result = result->ai_next;
	}

	//Last entry is nullptr, return
	if( result->ai_addr == nullptr )
	{
		freeaddrinfo( start );
		return nullptr;
	}

	std::shared_ptr<SocketAddress> sockAddr = std::make_shared<SocketAddress>( *( result->ai_addr ) );
	freeaddrinfo( start );
	return sockAddr;
}


//-------------------------------------------------------------------------------------------------
STATIC int NetworkUtils::ReportError( bool printLog /*= true*/ )
{
	int errorCode = WSAGetLastError( );
	if( printLog )
	{
		std::string errorLog;
		if( errorCode == 10048 )
		{
			errorLog = "Socket address already in use.";
		}
		else if( errorCode == WOULD_BLOCK_ERROR )
		{
			errorLog = "Resource temporarily unavailable. Non-blocking empty recv or connect.";
		}
		else if( errorCode == 10061 )
		{
			errorLog = "Connection refused. Address probably isn't hosting.";
		}
		else
		{
			errorLog = Stringf( "%s%d", "Network System error code: ", errorCode );
		}
		g_ConsoleSystem->AddLog( errorLog, Console::BAD );
	}
	return errorCode;
}


//-------------------------------------------------------------------------------------------------
STATIC UDPSocketPtr NetworkUtils::CreateUDPSocket( eSocketAddressFamily family )
{
	SOCKET newSocket = socket( family, SOCK_DGRAM, IPPROTO_IP );
	if( newSocket != INVALID_SOCKET )
	{
		return UDPSocketPtr( new UDPSocket( newSocket ) );
	}
	else
	{
		ReportError( );
		return nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC TCPSocketPtr NetworkUtils::CreateTCPSocket( eSocketAddressFamily family )
{
	SOCKET newSocket = socket( family, SOCK_STREAM, IPPROTO_IP );
	if( newSocket != INVALID_SOCKET )
	{
		return TCPSocketPtr( new TCPSocket( newSocket ) );
	}
	else
	{
		ReportError( );
		return nullptr;
	}
}



//-------------------------------------------------------------------------------------------------
STATIC char const * NetworkUtils::GetLocalHostName( )
{
	static char buffer[256];
	if( ::gethostname( buffer, 256 ) == 0 )
	{
		return buffer;
	}
	else
	{
		return "localhost";
	}
}


//-------------------------------------------------------------------------------------------------
// Old stuff
//-------------------------------------------------------------------------------------------------
// Get All Addresses that match our criteria
addrinfo * AllocAddressesForHost( char const * host,	// host, like google.com
	char const * service,								// service, usually the port number as a string
	int family,											// Connection Family, AF_INET (IPv4) for this assignment
	int socktype,										// Socket Type, SOCK_STREAM or SOCK_DGRAM (TCP or UDP) for this class
	int flags /*= 0*/ )									// Search flag hints, we use this for AI_PASSIVE (bindable addresses)
{
	// Also, family of AF_UNSPEC will return all address that support the 
	// sock type (so both IPv4 and IPv6 adddress).

	// Define the hints - this is what it will use
	// for determining what addresses to return
	addrinfo hints;
	memset( &hints, 0, sizeof( hints ) );

	hints.ai_family = family;
	hints.ai_socktype = socktype;
	hints.ai_flags = flags;

	// This will allocate all addresses into a single linked list
	// with the head put into result.
	addrinfo * result = nullptr;
	int status = getaddrinfo( host, service, &hints, &result );
	if( status != 0 )
	{
		// Warning( "net", "Failed to find addresses for [%s:%s]. Error[%s]", host, service, gai_strerror(status) );
		return nullptr;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
// get address part of a sockaddr, IPv4 or IPv6:
void * GetInAddr( sockaddr const * addr )
{
	//#TODO: Conversion to AF_INET6
	if( addr->sa_family == AF_INET )
	{
		return &( ( (sockaddr_in*) addr )->sin_addr );
	}
	else
	{
		return &( ( (sockaddr_in6*) addr )->sin6_addr );
	}
}


//-------------------------------------------------------------------------------------------------
bool SockAddrFromString( sockaddr_in * out_addr, char const * address )
{
	std::vector<std::string> splitAddress = SplitString( address, ':' );
	if( splitAddress.size( ) != 2 )
	{
		return false;
	}
	//#TODO: Look into changing this to AF_INET6
	addrinfo * addrList = AllocAddressesForHost( splitAddress[0].c_str(), splitAddress[1].c_str(), AF_INET, SOCK_DGRAM );
	if( !addrList )
	{
		return false;
	}
	*out_addr = *( (sockaddr_in*) addrList->ai_addr );
	FreeAddresses( addrList );
	return true;
}


//-------------------------------------------------------------------------------------------------
// Since we allocate the addresses, we must free them.
// Takes the addrinfo written to by getaddrinfo(...)
void FreeAddresses( addrinfo * addresses )
{
	if( nullptr != addresses )
	{
		freeaddrinfo( addresses );
	}
}


//-------------------------------------------------------------------------------------------------
// Ignoring Non-Critical Errors
// These errors are non-fatal and are more or less ignorable.
bool SocketErrorShouldDisconnect( int error )
{
	switch( error )
	{
	case WSAEWOULDBLOCK: // nothing to do - would've blocked if set to blocking
	case WSAEMSGSIZE:    // UDP message too large - ignore that packet.
	case WSAECONNRESET:  // Other side reset their connection.
		return false;

	default:
		return true;
	}
};


//-------------------------------------------------------------------------------------------------
bool IsEqual( sockaddr_in const & first, sockaddr_in const & second )
{
	return ( first.sin_addr.S_un.S_addr == second.sin_addr.S_un.S_addr ) && (first.sin_port == second.sin_port);
}


//-------------------------------------------------------------------------------------------------
void SetBlocking( SOCKET socket, bool isBlocking )
{
	u_long blocking = 0;
	if( !isBlocking )
	{
		blocking = 1;
	}
	ioctlsocket( socket, FIONBIO, &blocking );
}


//-------------------------------------------------------------------------------------------------
// Converting a sockaddr to a String
// Again, doing this in a global buffer so it is not thread safe
char const * StringFromSockAddr( sockaddr_in const * addr )
{
	static char buffer[256];

	// Hard coding this for sockaddr_in for brevity
	// You can make this work for IPv6 as well
	sockaddr * saddr = (sockaddr*) addr;

	// inet_ntop converts an address type to a human readable string,
	// ie 0x7f000001 => "127.0.0.1"
	// GetInAddr (defined below) gets the pointer to the address part of the sockaddr
	char hostname[256];
	inet_ntop( addr->sin_family, GetInAddr( saddr ), hostname, 256 );

	// Combine the above with the port.  
	// Port is stored in network order, so convert it to host order
	// using ntohs (Network TO Host Short)
	sprintf_s( buffer, 256, "%s:%u", hostname, ntohs( addr->sin_port ) );

	// buffer is static - so will not go out of scope, but that means this is not thread safe.
	return buffer;
}