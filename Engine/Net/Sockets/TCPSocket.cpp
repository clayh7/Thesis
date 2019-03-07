#include "Engine/Net/Sockets/TCPSocket.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Net/Sockets/SocketAddress.hpp"


//-------------------------------------------------------------------------------------------------
STATIC int const TCPSocket::MAX_BACKLOG = 32;


//-------------------------------------------------------------------------------------------------
TCPSocket::~TCPSocket( )
{
	closesocket( m_socket );
}


//-------------------------------------------------------------------------------------------------
int TCPSocket::Connect( SocketAddressPtr address )
{
	int error = connect( m_socket, &( address->m_sockAddr ), address->GetSize( ) );
	if( error == SOCKET_ERROR )
	{
		return NetworkUtils::ReportError( );
	}
	m_address = address;
	return NO_ERROR;
}


//-------------------------------------------------------------------------------------------------
int TCPSocket::Bind( SocketAddressPtr address )
{
	int error = bind( m_socket, &( address->m_sockAddr ), address->GetSize( ) );
	if( error == SOCKET_ERROR )
	{
		return NetworkUtils::ReportError( );
	}
	m_address = address;
	return NO_ERROR;
}


//-------------------------------------------------------------------------------------------------
int TCPSocket::Listen( int backlog )
{
	int error = listen( m_socket, backlog );
	if( error == SOCKET_ERROR )
	{
		return NetworkUtils::ReportError( );
	}
	return NO_ERROR;
}


//-------------------------------------------------------------------------------------------------
TCPSocketPtr TCPSocket::Accept( )
{
	sockaddr fromAddress;
	int fromLength = sizeof( fromAddress );
	SOCKET newSocket = accept( m_socket, &fromAddress, &fromLength );
	if( newSocket != INVALID_SOCKET )
	{
		TCPSocket * tcpSocket = new TCPSocket( newSocket );
		tcpSocket->SetAddress( fromAddress );
		return TCPSocketPtr( tcpSocket );
	}
	else
	{
		return nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
int TCPSocket::Send( void const * data, int length )
{
	int bytesSentCount = send( m_socket, static_cast<char const *>( data ), length, 0 );
	if( bytesSentCount < 0 )
	{
		NetworkUtils::ReportError( );
	}
	return bytesSentCount;
}


//-------------------------------------------------------------------------------------------------
int TCPSocket::Receive( void * buffer, int length )
{
	int bytesReceivedCount = recv( m_socket, static_cast<char*>( buffer ), length, 0 );
	if( bytesReceivedCount < 0 )
	{
		NetworkUtils::ReportError( false );
	}
	return bytesReceivedCount;
}


//-------------------------------------------------------------------------------------------------
void TCPSocket::SetBlocking( bool isBlocking )
{
	u_long blocking = isBlocking ? 0 : 1;
	int error = ioctlsocket( m_socket, FIONBIO, &blocking );
	if( error == SOCKET_ERROR )
	{
		NetworkUtils::ReportError( );
	}
}


//-------------------------------------------------------------------------------------------------
std::string TCPSocket::GetAddressString( ) const
{
	return m_address->GetAddress( );
}


//-------------------------------------------------------------------------------------------------
TCPSocket::TCPSocket( SOCKET inSocket )
	: m_socket( inSocket )
	, m_address( nullptr )
{
	SetBlocking( false );
}


//-------------------------------------------------------------------------------------------------
void TCPSocket::SetAddress( SocketAddress const & address )
{
	m_address = std::make_shared<SocketAddress>( address );
}