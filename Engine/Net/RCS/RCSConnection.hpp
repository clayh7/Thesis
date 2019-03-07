#pragma once

#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class TCPSocket;


//-------------------------------------------------------------------------------------------------
class RCSConnection
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static int const BUFFER_SIZE = 1024;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	TCPSocketPtr m_tcpSocket;
	char m_messageBuffer[BUFFER_SIZE];
	size_t m_messageBufferIndex;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	RCSConnection( TCPSocketPtr tcpSocket );
	~RCSConnection( );

	void Send( eRCSMessageType messageType, char const * message ) const;
	void Receive( );
	SOCKET GetSocket( ) const;
	std::string GetAddress( ) const;

private:
	void AddToBuffer( char readChar );
	void ClearMessageBuffer( );
	void CreateMessageEvent( );
};