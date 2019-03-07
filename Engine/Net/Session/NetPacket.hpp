#pragma once

#include <vector>
#include "Engine/Utils/BytePacker.hpp"
//#include "Engine/Net/NetworkSystem.hpp"
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class NetConnection;
class NetMessage;
class NetSession;


//-------------------------------------------------------------------------------------------------
class NetSender
{
public:
	NetSession * session;
	NetConnection * connection;
	sockaddr_in fromAddress;
};


//-------------------------------------------------------------------------------------------------
class PacketHeader
{
public:
	uint8_t fromConnIndex;
	uint16_t packetAck;
	uint16_t mostRecentReceivedAck;
	uint16_t previousReceivedAcksBitfield;
	uint8_t messageCount;

public:
	size_t const GetTotalWrittenHeaderSize( )
	{
		return sizeof( uint8_t ) * 2 + sizeof( uint16_t ) * 3;
	};
};


//-------------------------------------------------------------------------------------------------
class NetPacket : public BytePacker
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static size_t const MAX_SIZE = 1444;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	byte_t m_data[MAX_SIZE];

public:
	NetSender m_senderInfo;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	NetPacket( );

	void WriteHeader( PacketHeader * header );
	bool CanWriteMessage( NetMessage const * message ) const;
	bool WriteMessage( NetMessage const * message );

	void ReadHeader( PacketHeader * header ) const;

	size_t GetSize( ) const;
	NetPacket * Copy( ) const;
};