#pragma once


//-------------------------------------------------------------------------------------------------
typedef unsigned short uint16_t;


//-------------------------------------------------------------------------------------------------
class AckBundle
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static uint16_t const INVALID_ACK_ID = 65535;
	static size_t const MAX_RELIABLES_PER_PACKET = 256;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	uint16_t m_ackID;
	uint16_t m_attachedReliables[MAX_RELIABLES_PER_PACKET];
	size_t m_reliableMessageCount;
	double m_sentTimeStamp;
	bool m_confirmReceived;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	AckBundle( );
	AckBundle( uint16_t id );

	void AddReliableID( uint16_t reliableID );
};