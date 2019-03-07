#include "Engine/Net/Session/AckBundle.hpp"

#include "Engine/Core/Time.hpp"


//-------------------------------------------------------------------------------------------------
AckBundle::AckBundle( )
	: m_ackID( INVALID_ACK_ID )
	, m_reliableMessageCount( 0 )
	, m_confirmReceived( false )
	, m_sentTimeStamp( 0.0 )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
AckBundle::AckBundle( uint16_t id )
	: m_ackID( id )
	, m_reliableMessageCount( 0 )
	, m_confirmReceived( false )
	, m_sentTimeStamp( Time::TOTAL_SECONDS )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void AckBundle::AddReliableID( uint16_t reliableID )
{
	m_attachedReliables[m_reliableMessageCount] = reliableID;
	++m_reliableMessageCount;
}

