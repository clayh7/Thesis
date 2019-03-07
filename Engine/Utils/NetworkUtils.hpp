#pragma once

#pragma comment(lib, "ws2_32.lib") //For networking stuff
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <memory>
#include <string>


//-------------------------------------------------------------------------------------------------
class SocketAddress;
class TCPSocket;
class UDPSocket;
typedef std::shared_ptr<SocketAddress> SocketAddressPtr;
typedef std::shared_ptr<TCPSocket> TCPSocketPtr;
typedef std::shared_ptr<UDPSocket> UDPSocketPtr;


//-------------------------------------------------------------------------------------------------
enum eSocketAddressFamily
{
	eSocketAddressFamily_IPv4 = AF_INET,
	eSocketAddressFamily_IPv6 = AF_INET6,
};


//-------------------------------------------------------------------------------------------------
enum eRCSMessageType : char
{
	eRCSMessageType_END,
	eRCSMessageType_COMMAND,
	eRCSMessageType_ECHO,
	eRCSMessageType_RENAME,
	eRCSMessageType_ERROR,
};


//-------------------------------------------------------------------------------------------------
class NetworkUtils
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static int const WOULD_BLOCK_ERROR;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static SocketAddressPtr CreateIPv4FromString( std::string const & addressString );
	static SocketAddressPtr CreateIPv4FromString( std::string const & host, uint32_t port );
	static SocketAddressPtr CreateIPv4FromString( std::string const & host, std::string const & service );
	static TCPSocketPtr CreateTCPSocket( eSocketAddressFamily family );
	static UDPSocketPtr CreateUDPSocket( eSocketAddressFamily family );

	static char const * GetLocalHostName( );

	static int ReportError( bool printLog = true );
};


//-------------------------------------------------------------------------------------------------
// Old Stuff
//-------------------------------------------------------------------------------------------------
addrinfo * AllocAddressesForHost( char const * host, char const * service, int family, int socktype, int flags = 0 );								
void * GetInAddr( sockaddr const * addr );
bool SockAddrFromString( sockaddr_in * out_addr, char const * address );
void FreeAddresses( addrinfo * addresses );
bool SocketErrorShouldDisconnect( int error );
bool IsEqual( sockaddr_in const & first, sockaddr_in const & second );
void SetBlocking( SOCKET socket, bool isBlocking );
char const * StringFromSockAddr( sockaddr_in const * addr );