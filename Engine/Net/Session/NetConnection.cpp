#include "Engine/Net/Session/NetConnection.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Net/Session/NetSession.hpp"
#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC double NetConnection::s_resendDelaySeconds = 0.2; //Default 200ms


//-------------------------------------------------------------------------------------------------
NetConnection::NetConnection( unsigned char index, sockaddr_in const & address, std::string const & guid, std::string const & username, NetSession const * const session )
	: m_connectionInfo( index, address, guid.c_str( ), username.c_str( ) )
	, m_session( session )
	, m_timeLastSent( Time::TOTAL_SECONDS )
	, m_timeLastRecv( Time::TOTAL_SECONDS )
	, m_dropsCounted( 0 )
	, m_receivedCounted( 0 )
	, m_nextToSendAck( 0 )
	, m_mostRecentReceivedAck( AckBundle::INVALID_ACK_ID )
	, m_mostRecentReceivedAcksBitfield( 0 )
	, m_nextToSendReliableID( 0 )
	, m_nextUnconfirmedReliableID( 0 )
	, m_oldestUnconfirmedReliableID( 0 )
	, m_nextUnreceivedReliableID( 0 )
	, m_oldestUnreceivedReliableID( 0 )
	, m_lastJoinRequestNuonce( (uint32_t) -1 )
	, m_roundTripTime( s_resendDelaySeconds )
{
	//Initialize confirmed reliable IDs
	for( size_t confirmIndex = 0; confirmIndex < MAX_RELIABLE_RANGE; ++confirmIndex )
	{
		m_confirmedReliableIDs[confirmIndex] = NetMessage::INVALID_RELIABLE_ID;
	}

	//Initialize received reliable IDs
	for( size_t receivedIndex = 0; receivedIndex < MAX_RELIABLE_RANGE; ++receivedIndex )
	{
		m_receivedReliableIDs[receivedIndex] = NetMessage::INVALID_RELIABLE_ID;
	}

	//Initialize sequence IDs
	for( size_t channelIndex = 0; channelIndex < MAX_SEQUENCE_CHANNELS; ++channelIndex )
	{
		m_nextToSendSequenceID[channelIndex] = 0;
	}
	for( size_t channelIndex = 0; channelIndex < MAX_SEQUENCE_CHANNELS; ++channelIndex )
	{
		m_nextToReceiveReliableSequenceID[channelIndex] = 0;
	}
	
	//Initialize sequence channels
	for( size_t channelIndex = 0; channelIndex < MAX_SEQUENCE_CHANNELS; ++channelIndex )
	{
		m_sequenceChannels[channelIndex] = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
NetConnection::~NetConnection( )
{
	//Destroy all remaining unreliables
	while( m_unsentUnreliableMessages.size( ) > 0 )
	{
		NetMessage const * message = m_unsentUnreliableMessages.front( );
		delete message;
		m_unsentUnreliableMessages.pop( );
	}

	//Destroy all remaining reliables
	while( m_unsentReliableMessages.size( ) > 0 )
	{
		NetMessage const * message = m_unsentReliableMessages.front( );
		delete message;
		m_unsentReliableMessages.pop( );
	}
	while( m_sentReliableMessages.size( ) > 0 )
	{
		NetMessage const * message = m_sentReliableMessages.front( );
		delete message;
		m_sentReliableMessages.pop( );
	}

	//Destroy all messages in channels
	for( size_t channelIndex = 0; channelIndex < MAX_SEQUENCE_CHANNELS; ++channelIndex )
	{
		while( m_sequenceChannels[channelIndex] )
		{
			NetMessage * deleteMe = m_sequenceChannels[channelIndex];
			m_sequenceChannels[channelIndex] = m_sequenceChannels[channelIndex]->m_next;
			delete deleteMe;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::AddMessage( NetMessage & message )
{
	//Set index
	if( m_session )
	{
		if( m_session->GetSelf( ) )
		{
			message.m_senderIndex = m_session->GetSelf( )->GetIndex( );
		}
	}
	
	//Set Definition
	NetMessageDefinition const * def = m_session->GetDefinition( message.GetNetMessageType( ) );
	message.m_definition = def;

	//Attach sequence ID
	if( def->IsReliable( ) && def->IsSequence( ) )
	{
		message.m_sequenceID = GetSequenceID( def->sequenceChannelID );
	}

	//Check where it belongs
	if( def->IsReliable( ) )
	{
		m_unsentReliableMessages.push( message.Copy( ) );
	}
	else
	{
		m_unsentUnreliableMessages.push( message.Copy( ) );
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::AddBundle( AckBundle const & bundle )
{
	//Get bundle slot
	size_t slot = bundle.m_ackID % MAX_ACK_BUNDLES;

	//Track connection stats
	if( m_bundles[slot].m_ackID != AckBundle::INVALID_ACK_ID )
	{
		//Track drop rate
		if( m_bundles[slot].m_confirmReceived )
		{
			++m_receivedCounted;
		}
		else
		{
			++m_dropsCounted;
		}

		//Reset them if they get too high
		if( m_dropsCounted > DROP_COUNT_RESET_VALUE || m_receivedCounted > DROP_COUNT_RESET_VALUE )
		{
			m_receivedCounted /= 2;
			m_dropsCounted /= 2;
		}
	}

	//Store new bundle
	m_bundles[slot] = bundle;
}


//-------------------------------------------------------------------------------------------------
void NetConnection::SendPacket( )
{
	if( !m_session->IsConnected( ) )
	{
		return;
	}

	//Heartbeat is a packet with no messages
	bool heartbeat = IsTimeForHeartbeat( );

	//Check if we need to send heartbeat
	if( !heartbeat )
	{
		//We have a bad connection
		if( IsBad( ) )
		{
			return;
		}
		//If there is nothing to send, don't continue
		else if( m_unsentUnreliableMessages.size( ) == 0 &&
			m_unsentReliableMessages.size() == 0 &&
			m_sentReliableMessages.size() == 0 )
		{
		
			return;
		}
	}

	//Keep sending packets until we have no more data to send, up to MAX_PACKET_SEND_AMOUNT_PER_CONNECTION
	m_lastOutgoingPackageCount = 0;
	m_lastOutgoingMessageCount = 0;
	m_lastOutgoingByteCountPacketHeader = 0;
	m_lastOutgoingByteCountMessages = 0;
	for( int numberOfPacketsSent = 0; numberOfPacketsSent < NetSession::MAX_PACKET_SEND_AMOUNT_PER_CONNECTION; ++numberOfPacketsSent )
	{
		//Setup packet Header
		PacketHeader header;
		header.fromConnIndex = m_connectionInfo.m_index;
		header.packetAck = GetNextAck( );
		header.mostRecentReceivedAck = m_mostRecentReceivedAck;
		header.previousReceivedAcksBitfield = m_mostRecentReceivedAcksBitfield;
		AckBundle bundle( header.packetAck );
		NetSession::m_NetworkTrafficActivity.Printf( "Sending Packet (%u) to Connection %u", header.packetAck, m_connectionInfo.m_index );

		//Create packet
		NetPacket packet;
		//Note: Does not write the number of messages
		packet.WriteHeader( &header );
		//Save spot to write number of messages
		size_t numMessagesBookmark = packet.Reserve<uint8_t>( 0U );

		//Update sent time stamp
		m_timeLastSent = Time::TOTAL_SECONDS;

		//Immediately send heartbeat, and don't send anything else
		if( heartbeat )
		{
			//Tracking for debugging
			m_lastOutgoingPackageCount = 1;
			m_lastOutgoingMessageCount = 0;
			m_lastOutgoingByteCountMessages = 0;
			m_lastOutgoingByteCountPacketHeader = header.GetTotalWrittenHeaderSize( );
			m_session->SendPacket( m_connectionInfo.m_address, packet.GetBuffer( ), packet.GetSize( ) );
			break;
		}
		else
		{
			size_t count = 0;
			size_t bytesWritten;
			size_t totalBytesWritten = 0;
			count += WriteSentReliables( &packet, &bundle, &bytesWritten );
			totalBytesWritten += bytesWritten;
			count += WriteUnsentReliables( &packet, &bundle, &bytesWritten );
			totalBytesWritten += bytesWritten;
			count += WriteUnsentUnreliables( &packet, &bytesWritten );
			totalBytesWritten += bytesWritten;

			//We have no more messages to send, but we will send at least one to keep the connection
			if( count == 0 && numberOfPacketsSent > 0 )
			{
				break;
			}
			else
			{
				//Tracking for debugging
				m_lastOutgoingPackageCount++;
				m_lastOutgoingMessageCount += count;
				m_lastOutgoingByteCountMessages += totalBytesWritten;
				m_lastOutgoingByteCountPacketHeader += header.GetTotalWrittenHeaderSize( );
			}

			//Update message count
			packet.WriteAt<uint8_t>( numMessagesBookmark, (uint8_t)count );
			m_session->SendPacket( m_connectionInfo.m_address, packet.GetBuffer( ), packet.GetSize( ) );
		}
		AddBundle( bundle );
	}

	CleanUpRemainingUnreliables( );
}


//-------------------------------------------------------------------------------------------------
size_t NetConnection::WriteSentReliables( NetPacket * packet, AckBundle * bundle, size_t * out_bytesWritten )
{
	//Keep track of how many we are writing
	size_t messageCount = 0;
	*out_bytesWritten = 0;
	while( !m_sentReliableMessages.empty( ) )
	{
		NetMessage * message = m_sentReliableMessages.front( );

		//Delete confirmed reliables
		if( IsReliableIDConfirmed( message->m_reliableID ) )
		{
			m_sentReliableMessages.pop( );
			delete message;
			continue;
		}

		//Resend old reliables
		if( IsMessageOld( message ) )
		{
			if( packet->WriteMessage( message ) )
			{
				++messageCount;
				*out_bytesWritten += message->GetTotalWrittenMessageSize( );

				m_sentReliableMessages.pop( );
				bundle->AddReliableID( message->m_reliableID );
				message->m_sentTimeStamp = Time::TOTAL_SECONDS;
				m_sentReliableMessages.push( message );
			}
			else
			{
				//Unable to write message, time to return
				break;
			}
		}

		//If there are no more old reliables, return
		else
		{
			break;
		}
	}

	return messageCount;
}


//-------------------------------------------------------------------------------------------------
size_t NetConnection::WriteUnsentReliables( NetPacket * packet, AckBundle * bundle, size_t * out_bytesWritten )
{
	//Keep track of how many we are writing
	uint8_t messageCount = 0;
	*out_bytesWritten = 0;
	while( !m_unsentReliableMessages.empty( ) && CanSendNewReliables( ) )
	{
		NetMessage * message = m_unsentReliableMessages.front( );

		if( packet->CanWriteMessage( message ) )
		{
			++messageCount;
			*out_bytesWritten += message->GetTotalWrittenMessageSize( );

			//Assign next ID
			message->m_reliableID = GetNextReliableID( );
			bundle->AddReliableID( message->m_reliableID );
			packet->WriteMessage( message );

			//Remove from unsent
			m_unsentReliableMessages.pop( );

			//Add to sent
			message->m_sentTimeStamp = Time::TOTAL_SECONDS;
			m_sentReliableMessages.push( message );
		}
		else
		{
			break;
		}
	}
	return messageCount;
}


//-------------------------------------------------------------------------------------------------
size_t NetConnection::WriteUnsentUnreliables( NetPacket * packet, size_t * out_bytesWritten )
{
	//Keep track of how many we are writing
	size_t messageCount = 0;
	*out_bytesWritten = 0;
	while( m_unsentUnreliableMessages.size() > 0 )
	{
		//Get Message and Definition
		NetMessage const * message = m_unsentUnreliableMessages.front( );
		if( packet->WriteMessage( message ) )
		{
			++messageCount;
			*out_bytesWritten += message->GetTotalWrittenMessageSize( );

			m_unsentUnreliableMessages.pop( );
			delete message;
			message = nullptr;
		}
		else
		{
			break;
		}
	}

	return messageCount;
}


//-------------------------------------------------------------------------------------------------
void NetConnection::CleanUpRemainingUnreliables( )
{
	while( m_unsentUnreliableMessages.size( ) > 0 )
	{
		NetMessage const * message = m_unsentUnreliableMessages.front( );
		delete message;
		m_unsentUnreliableMessages.pop( );
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::ProcessMessage( NetSender const & sender, NetMessage const & message )
{
	//Reliable
	if( message.m_definition->IsReliable( ) )
	{
		//Return true if we need to process execute process
		if( MarkMessageReceived( message ) )
		{
			if( message.m_definition->IsSequence( ) )
			{
				ProcessReliableSequence( sender, message );
			}
			else
			{
				message.Process( sender );
			}
		}

	}
	//Unreliable
	else
	{
		if( message.m_definition->IsSequence( ) )
		{
			ProcessUnreliableSequence( sender, message );
		}
		else
		{
			message.Process( sender );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::ProcessReliableSequence( NetSender const & sender, NetMessage const & message )
{
	byte_t channelID = message.m_definition->sequenceChannelID;
	if( m_nextToReceiveReliableSequenceID[channelID] == message.m_sequenceID )
	{
		message.Process( sender );
		++m_nextToReceiveReliableSequenceID[channelID];
		ProcessNextInSequenceChannel( sender, channelID );
	}
	else
	{
		AddMessageToSequenceChannel( message, channelID );
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::ProcessNextInSequenceChannel( NetSender const & sender, byte_t sequenceChannelID )
{
	while( m_sequenceChannels[sequenceChannelID] )
	{
		if( m_sequenceChannels[sequenceChannelID]->m_sequenceID == m_nextToReceiveReliableSequenceID[sequenceChannelID] )
		{
			m_sequenceChannels[sequenceChannelID]->Process( sender );
			NetMessage * deleteMe = m_sequenceChannels[sequenceChannelID];
			m_sequenceChannels[sequenceChannelID] = m_sequenceChannels[sequenceChannelID]->m_next;
			delete deleteMe;
			deleteMe = nullptr;
			if( m_sequenceChannels[sequenceChannelID] )
			{
				m_sequenceChannels[sequenceChannelID]->m_prev = nullptr;
			}
			++m_nextToReceiveReliableSequenceID[sequenceChannelID];
		}
		else
		{
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::AddMessageToSequenceChannel( NetMessage const & message, byte_t sequenceChannelID )
{
	NetMessage * messageToAdd = message.Copy( );
	NetMessage * channelIter = m_sequenceChannels[sequenceChannelID];

	//if empty, make this head
	if( channelIter == nullptr )
	{
		m_sequenceChannels[sequenceChannelID] = messageToAdd;
	}
	else
	{
		while( channelIter )
		{
			//if this is smaller, insert
			if( GreaterThanCycle( channelIter->m_sequenceID, messageToAdd->m_sequenceID ) )
			{
				messageToAdd->m_next = channelIter;
				messageToAdd->m_prev = channelIter->m_prev;
				channelIter->m_prev = messageToAdd;
				if( messageToAdd->m_prev )
				{
					messageToAdd->m_prev->m_next = messageToAdd;
				}
				
				//if this was head, update head
				if( channelIter == m_sequenceChannels[sequenceChannelID] )
				{
					m_sequenceChannels[sequenceChannelID] = messageToAdd;
				}
				break;
			}
			else
			{
				//if reach end, add to back
				if( channelIter->m_next == nullptr )
				{
					channelIter->m_next = messageToAdd;
					messageToAdd->m_prev = channelIter;
					break;
				}
				else
				{
					channelIter = channelIter->m_next;
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::ProcessUnreliableSequence( NetSender const & sender, NetMessage const & message )
{
	//#TODO: Make sure this is okay
	if( message.m_ackID >= m_mostRecentReceivedAck )
	{
		message.Process( sender );
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::SetPassword( size_t password )
{
	m_connectionInfo.m_password = password;
}


//-------------------------------------------------------------------------------------------------
void NetConnection::UpdateLastRecvTime( )
{
	//Mark time received
	m_timeLastRecv = Time::TOTAL_SECONDS;
}


//-------------------------------------------------------------------------------------------------
void NetConnection::MarkPacketReceived( PacketHeader const & header )
{
	MarkAckReceived( header.packetAck );
	ConfirmAcksSent( header.mostRecentReceivedAck, header.previousReceivedAcksBitfield );
	UpdateLastRecvTime( );
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::MarkAckReceived( uint16_t ackID )
{
	//Duplicate ack
	if( ackID == m_mostRecentReceivedAck )
	{
		return false;
	}

	//Nothing to mark
	if( ackID == AckBundle::INVALID_ACK_ID )
	{
		return false;
	}

	//Special case - first ack report
	if( m_mostRecentReceivedAck == AckBundle::INVALID_ACK_ID )
	{
		m_mostRecentReceivedAck = ackID;
		m_mostRecentReceivedAcksBitfield = 0;
	}
	else if( GreaterThanCycle( ackID, m_mostRecentReceivedAck ) )
	{
		uint16_t shift = ackID - m_mostRecentReceivedAck;
		if( shift > 16 ) //number of bits in uint16_t
		{
			m_mostRecentReceivedAcksBitfield = 0;
		}
		else
		{
			m_mostRecentReceivedAcksBitfield = (m_mostRecentReceivedAcksBitfield << shift);
		}
		m_mostRecentReceivedAck = ackID;
		SetBit( &m_mostRecentReceivedAcksBitfield, shift - 1 );
	}
	else
	{
		uint16_t offset = m_mostRecentReceivedAck - ackID;
		SetBit( &m_mostRecentReceivedAcksBitfield, offset - 1 );
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void NetConnection::ConfirmAcksSent( uint16_t highestAck, uint16_t ackBitfield )
{
	ConfirmAckBundle( highestAck );
	for( uint16_t bitIndex = 0; bitIndex < 16; ++bitIndex)
	{
		if( IsBitfieldSet( ackBitfield, BIT( bitIndex ) ) )
		{
			ConfirmAckBundle( highestAck - bitIndex - 1 );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::ConfirmAckBundle( uint16_t ack )
{
	AckBundle & bundle = m_bundles[ack % MAX_ACK_BUNDLES];

	//Already confirmed (it's initialized to false)
	if( bundle.m_confirmReceived )
	{
		return;
	}

	//Correct ack bundle (we reuse the memory space)
	if( bundle.m_ackID != ack )
	{
		return;
	}

	//Confirm all reliable IDs attached
	for( size_t reliableIndex = 0; reliableIndex < bundle.m_reliableMessageCount; ++reliableIndex )
	{
		ConfirmReliableID( bundle.m_attachedReliables[reliableIndex] );
	}

	//Track round trip time
	double oldRTT = GetRoundTripTime( );
	double newRTT = Time::TOTAL_SECONDS - bundle.m_sentTimeStamp;
	m_roundTripTime = ( 0.9 ) * oldRTT + ( 0.1 ) * newRTT;
	bundle.m_confirmReceived = true;
}


//-------------------------------------------------------------------------------------------------
void NetConnection::ConfirmReliableID( uint16_t reliableID )
{
	// waaay too far ahead. Stop.
	if( GreaterThanCycle( reliableID, m_nextUnconfirmedReliableID + MAX_RELIABLE_RANGE ) )
	{
		ASSERT_RECOVERABLE( false, "Trying to confirm a reliable message above our available range" );
		return;
	}

	if( GreaterThanCycle( reliableID, m_nextUnconfirmedReliableID ) || reliableID == m_nextUnconfirmedReliableID )
	{
		//clearing values along the way because of edge case where same value is skipped multiple times
		uint16_t clearID = m_nextUnconfirmedReliableID;
		while( clearID != reliableID )
		{
			m_confirmedReliableIDs[clearID % MAX_RELIABLE_RANGE] = NetMessage::INVALID_RELIABLE_ID;
			++clearID;
		}

		m_nextUnconfirmedReliableID = reliableID + 1;

		//Be sure to cycle next expected confirmed reliable message
		if( m_nextUnconfirmedReliableID == NetMessage::INVALID_RELIABLE_ID )
		{
			++m_nextUnconfirmedReliableID;
		}
	}
	
	// We don't need to mark it if it's below range, it's assumed to be received
	if( GreaterThanCycle( m_nextUnconfirmedReliableID - MAX_RELIABLE_RANGE, reliableID ) )
	{
		//Nothing
	}
	else
	{
		//confirm reliable ID
		m_confirmedReliableIDs[reliableID % MAX_RELIABLE_RANGE] = reliableID;
	}

	UpdateOldestUnconfirmedReliableID( );
}


//-------------------------------------------------------------------------------------------------
void NetConnection::UpdateOldestUnconfirmedReliableID( )
{
	//Bring oldest up to minimum if below
	if( GreaterThanCycle( m_nextUnconfirmedReliableID - MAX_RELIABLE_RANGE, m_oldestUnconfirmedReliableID ) )
	{
		m_oldestUnconfirmedReliableID = m_nextUnconfirmedReliableID - MAX_RELIABLE_RANGE;
	}

	//Raise oldest until oldest is found or we catch up to next
	while( m_oldestUnconfirmedReliableID != m_nextUnconfirmedReliableID && IsReliableIDConfirmed( m_oldestUnconfirmedReliableID ) )
	{
		++m_oldestUnconfirmedReliableID;
	}
}


//-------------------------------------------------------------------------------------------------
void NetConnection::UpdateOldestUnreceivedReliableID( )
{
	//Raise oldest until oldest is found or we catch up to next
	while( m_oldestUnreceivedReliableID != m_nextUnreceivedReliableID && IsReliableIDReceived( m_oldestUnreceivedReliableID ) )
	{
		++m_oldestUnreceivedReliableID;
	}
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::MarkMessageReceived( NetMessage const & message )
{
	if( message.m_definition->IsReliable( ) )
	{
		uint16_t reliableID = message.m_reliableID;

		//Is outside bottom range
		if( GreaterThanCycle( m_nextUnreceivedReliableID - MAX_RELIABLE_RANGE, reliableID ) )
		{
			return false;
		}
		
		//Is outside top range
		if( GreaterThanCycle( reliableID, m_nextUnreceivedReliableID + MAX_RELIABLE_RANGE ) )
		{
			ASSERT_RECOVERABLE( false, "Received a reliable ID above max range" );
			return false;
		}
		
		//Has already been received?
		if( IsReliableIDReceived( reliableID ) )
		{
			return false;
		}
		//Has not been received
		else
		{
			//Is it above expected
			if( GreaterThanCycle( reliableID, m_nextUnreceivedReliableID ) || reliableID == m_nextUnreceivedReliableID )
			{
				//Is new reliable message id > oldest reliable id + MAX_RELIABLE_RANGE?
				if( GreaterThanCycle( reliableID, m_oldestUnreceivedReliableID + MAX_RELIABLE_RANGE ) )
				{
					ASSERT_RECOVERABLE( false, "Received a reliable ID too far in advanced" );
					return false;
				}

				m_nextUnreceivedReliableID = reliableID + 1;
			}

			//Add NEW reliable ID to received
			m_receivedReliableIDs[reliableID % MAX_RELIABLE_RANGE] = reliableID;

			UpdateOldestUnreceivedReliableID( );
		}
	}
	else
	{
		//Nothing
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
float NetConnection::GetDropRate( ) const
{
	if( m_dropsCounted + m_receivedCounted == 0 )
	{
		return 0.f;
	}
	return (float) ( m_dropsCounted ) / (float) ( m_dropsCounted + m_receivedCounted );
}


//-------------------------------------------------------------------------------------------------
double NetConnection::GetRoundTripTime( ) const
{
	return m_roundTripTime;
}


//-------------------------------------------------------------------------------------------------
byte_t NetConnection::GetIndex( ) const
{
	return m_connectionInfo.m_index;
}


//-------------------------------------------------------------------------------------------------
char const * NetConnection::GetGUID( ) const
{
	return m_connectionInfo.m_guid;
}


//-------------------------------------------------------------------------------------------------
char const * NetConnection::GetUsername( ) const
{
	return m_connectionInfo.m_username;
}


//-------------------------------------------------------------------------------------------------
size_t NetConnection::GetPassword( ) const
{
	return m_connectionInfo.m_password;
}


//-------------------------------------------------------------------------------------------------
sockaddr_in const & NetConnection::GetAddress( ) const
{
	return m_connectionInfo.m_address;
}


//-------------------------------------------------------------------------------------------------
NetSession const * const NetConnection::GetSession( ) const
{
	return m_session;
}


//-------------------------------------------------------------------------------------------------
uint16_t NetConnection::GetNextAck( )
{
	uint16_t ack = m_nextToSendAck;
	++m_nextToSendAck;
	if( m_nextToSendAck == AckBundle::INVALID_ACK_ID )
	{
		++m_nextToSendAck;
	}
	return ack;
}


//-------------------------------------------------------------------------------------------------
uint16_t NetConnection::GetNextReliableID( )
{
	uint16_t reliableID = m_nextToSendReliableID;
	++m_nextToSendReliableID;
	if( m_nextToSendReliableID == NetMessage::INVALID_RELIABLE_ID )
	{
		++m_nextToSendReliableID;
	}
	return reliableID;
}


//-------------------------------------------------------------------------------------------------
uint16_t NetConnection::GetSequenceID( byte_t sequenceChannelID )
{
	uint16_t id = m_nextToSendSequenceID[sequenceChannelID];
	++m_nextToSendSequenceID[sequenceChannelID];
	//There are no invalid sequence ids
	return id;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::IsSelf( ) const
{
	return m_session->GetSelf( ) == this;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::IsTimeForHeartbeat( ) const
{
	return ( Time::TOTAL_SECONDS - m_timeLastSent ) > (double) NetSession::HEARTBEAT_INTERVAL_SECONDS;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::IsBad( ) const
{
	return ( Time::TOTAL_SECONDS - m_timeLastRecv ) > (double) NetSession::BAD_CONNECTION_INTERVAL_SECONDS;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::IsReliableIDReceived( uint16_t reliableID ) const
{
	if( GreaterThanCycle( m_nextUnreceivedReliableID - MAX_RELIABLE_RANGE, reliableID ) )
	{
		return true;
	}

	if( m_receivedReliableIDs[reliableID % MAX_RELIABLE_RANGE] == reliableID )
	{
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::IsReliableIDConfirmed( uint16_t reliableID ) const
{
	if( GreaterThanCycle( m_nextUnconfirmedReliableID - MAX_RELIABLE_RANGE, reliableID ) )
	{
		return true;
	}

	if( m_confirmedReliableIDs[reliableID % MAX_RELIABLE_RANGE] == reliableID )
	{
		return true;
	}

	return false;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::IsMessageOld( NetMessage const * message ) const
{
	return ( Time::TOTAL_SECONDS - message->m_sentTimeStamp ) > GetRoundTripTime( ) * 1.1;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::IsHost( ) const
{
	return m_session->GetHost( ) == this;
}


//-------------------------------------------------------------------------------------------------
bool NetConnection::CanSendNewReliables( ) const
{
	return ( m_nextToSendReliableID - m_oldestUnconfirmedReliableID ) < MAX_RELIABLE_RANGE;
}


//-------------------------------------------------------------------------------------------------
void NetConnection::SetConnectionInfo( ConnectionInfo const & connInfo )
{
	m_connectionInfo = connInfo;
}