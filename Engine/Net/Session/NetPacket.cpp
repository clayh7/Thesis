#include "Engine/Net/Session/NetPacket.hpp"

#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Net/Session/NetSession.hpp"


//-------------------------------------------------------------------------------------------------
NetPacket::NetPacket( )
	: BytePacker( m_data, MAX_SIZE, 0 )
	, m_senderInfo( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void NetPacket::WriteHeader( PacketHeader * header )
{
	Write<uint8_t>( header->fromConnIndex );
	Write<uint16_t>( header->packetAck );
	Write<uint16_t>( header->mostRecentReceivedAck );
	Write<uint16_t>( header->previousReceivedAcksBitfield );
}


//-------------------------------------------------------------------------------------------------
bool NetPacket::CanWriteMessage( NetMessage const * message ) const
{
	NetMessageDefinition const * definition = message->m_definition;

	//Trying to send a message that we dont have a definition for
	if( !definition )
	{
		ERROR_AND_DIE( "Message Definition not registered" );
	}

	size_t totalSize = message->GetTotalWrittenMessageSize( );

	//Make sure we can write this much
	if( GetWritableBytesLeft( ) >= totalSize )
	{
		//There is room left
		return true;
	}
	else
	{
		//No more messages can be written
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
bool NetPacket::WriteMessage( NetMessage const * message )
{
	if( CanWriteMessage( message ) )
	{
		NetMessageDefinition const * definition = message->m_definition;
		size_t messageSize = definition->headerSize + message->GetPayloadSize( );

		//size of message
		Write<uint16_t>( (uint16_t) messageSize );

		//header : type
		Write<uint8_t>( (uint8_t) definition->type );

		//header : sender index
		if( !definition->IsConnectionless( ) )
		{
			Write<uint8_t>( (uint8_t) message->m_senderIndex );
		}

		//header : reliable ID
		if( definition->IsReliable( ) )
		{
			Write<uint16_t>( (uint16_t) message->m_reliableID );
		}

		//header : sequence ID
		if( definition->IsReliable( ) && definition->IsSequence( ) )
		{
			Write<uint16_t>( (uint16_t) message->m_sequenceID );
		}

		//payload
		WriteForward( message->GetBuffer( ), message->GetPayloadSize( ) );

		if( definition->IsReliable( ) && definition->IsSequence( ) )
		{
			NetSession::m_NetworkTrafficActivity.Printf( "\tMessage (type=%3u) (reliableID=%u) (sequenceID=%u) %4u Byte(s) ", definition->type, message->m_reliableID, message->m_sequenceID, message->GetTotalWrittenMessageSize( ) );
		}
		else if( definition->IsSequence( ) )
		{
			NetSession::m_NetworkTrafficActivity.Printf( "\tMessage (type=%3u) (sequenceID=%u) %4u Byte(s) ", definition->type, message->m_sequenceID, message->GetTotalWrittenMessageSize( ) );
		}
		else if( definition->IsReliable( ) )
		{
			NetSession::m_NetworkTrafficActivity.Printf( "\tMessage (type=%3u) (reliableID=%u) %4u Byte(s) ", definition->type, message->m_reliableID, message->GetTotalWrittenMessageSize( ) );
		}
		else
		{
			NetSession::m_NetworkTrafficActivity.Printf( "\tMessage (type=%3u) %4u Byte(s) ", definition->type, message->GetTotalWrittenMessageSize( ) );
		}
		return true;
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
void NetPacket::ReadHeader( PacketHeader * header ) const
{
	Read<uint8_t>( &( header->fromConnIndex ) );
	Read<uint16_t>( &( header->packetAck ) );
	Read<uint16_t>( &( header->mostRecentReceivedAck ) );
	Read<uint16_t>( &( header->previousReceivedAcksBitfield ) );
	Read<uint8_t>( &( header->messageCount ) );
}


//-------------------------------------------------------------------------------------------------
size_t NetPacket::GetSize( ) const
{
	return m_bufferSize;
}


//-------------------------------------------------------------------------------------------------
NetPacket * NetPacket::Copy( ) const
{
	NetPacket * copy = new NetPacket( );
	copy->m_senderInfo = m_senderInfo;
	copy->WriteForward( m_data, m_bufferSize );
	copy->Rewind( );
	return copy;
}