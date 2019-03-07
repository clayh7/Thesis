#pragma once

#include "Engine/Net/Session/NetMessageDefinition.hpp"
#include "Engine/Utils/BytePacker.hpp"


//-------------------------------------------------------------------------------------------------
class NetSender;


//-------------------------------------------------------------------------------------------------
class NetMessage : public BytePacker
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static size_t const MAX_SIZE = 1024;

public:
	static uint16_t const INVALID_RELIABLE_ID = 65535;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	double m_sentTimeStamp;
	NetMessageDefinition const * m_definition;
	byte_t m_type;
	uint16_t m_reliableID;
	uint16_t m_sequenceID;
	uint16_t m_ackID;
	byte_t m_senderIndex;
	NetMessage * m_next;
	NetMessage * m_prev;

private:
	byte_t m_data[MAX_SIZE];

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	NetMessage( byte_t type = eNetMessageType_INVALID );
	NetMessage( byte_t type, byte_t senderIndex );
	NetMessage( byte_t * buffer, size_t bufferSize );

	void Process( NetSender const & senderInfo ) const;

	size_t GetPayloadSize( ) const;
	size_t GetTotalWrittenMessageSize( ) const;
	eNetMessageType GetNetMessageType( ) const;
	NetMessage * Copy( ) const;
};