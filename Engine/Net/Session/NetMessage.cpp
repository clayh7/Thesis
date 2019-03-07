#include "Engine/Net/Session/NetMessage.hpp"

//include for NetSender
#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Engine/Net/Session/NetSession.hpp"


//-------------------------------------------------------------------------------------------------
NetMessage::NetMessage( byte_t type /*= eNetMessageType_INVALID*/ )
	: NetMessage( type, NetSession::INVALID_INDEX )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
NetMessage::NetMessage( byte_t type, byte_t senderIndex )
	: BytePacker( m_data, MAX_SIZE, 0 )
	, m_sentTimeStamp( 0.0 )
	, m_definition( nullptr )
	, m_type( (byte_t) type )
	, m_reliableID( INVALID_RELIABLE_ID )
	, m_sequenceID( 0 )
	, m_senderIndex( senderIndex )
	, m_next( nullptr )
	, m_prev( nullptr )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
NetMessage::NetMessage( byte_t * buffer, size_t bufferSize )
	: BytePacker( m_data, MAX_SIZE, bufferSize )
	, m_sentTimeStamp( 0.0 )
	, m_definition( nullptr )
	, m_type( (byte_t) -1 )
	, m_reliableID( INVALID_RELIABLE_ID )
	, m_sequenceID( 0 )
	, m_senderIndex( NetSession::INVALID_INDEX )
	, m_next( nullptr )
	, m_prev( nullptr )
{
	memcpy( m_data, buffer, bufferSize );
	m_buffer = m_data;
}


//-------------------------------------------------------------------------------------------------
void NetMessage::Process( NetSender const & senderInfo ) const
{
	m_definition->callback( senderInfo, *this );
}


//-------------------------------------------------------------------------------------------------
size_t NetMessage::GetPayloadSize( ) const
{
	return (size_t) m_bufferSize;
}


//-------------------------------------------------------------------------------------------------
size_t NetMessage::GetTotalWrittenMessageSize( ) const
{
	//Get how large our message is going to be
	size_t messageSize = m_definition->headerSize + GetPayloadSize( );

	//Includes data that says how big the message is
	size_t totalSize = messageSize + sizeof( uint16_t );

	return totalSize;
}


//-------------------------------------------------------------------------------------------------
eNetMessageType NetMessage::GetNetMessageType( ) const
{
	return (eNetMessageType) m_type;
}


//-------------------------------------------------------------------------------------------------
NetMessage * NetMessage::Copy( ) const
{
	NetMessage * copy = new NetMessage( *this );
	copy->m_buffer = copy->m_data;
	return copy;
}
