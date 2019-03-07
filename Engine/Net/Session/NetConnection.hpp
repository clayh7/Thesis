#pragma once

#include <queue>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Net/NetworkSystem.hpp"
#include "Engine/Net/Session/AckBundle.hpp"
#include "Engine/Net/Session/ConnectionInfo.hpp"


//-------------------------------------------------------------------------------------------------
class NetSession;
class NetMessage;
class NetPacket;
class NetSender;
class PacketHeader;


//-------------------------------------------------------------------------------------------------
class NetConnection
{
//-------------------------------------------------------------------------------------------------
// Statics
//-------------------------------------------------------------------------------------------------
public:
	static size_t const MAX_RELIABLE_RANGE = 1024;
	static size_t const MAX_ACK_BUNDLES = 128;
	//range that we care about for sequenced unreliable messages (any father and drop it)
	static size_t const MAX_UNRELIABLE_SEQUENCE_RANGE = 256;
	//range of Byte (data type of sequence ID)
	static size_t const MAX_SEQUENCE_CHANNELS = 256;
	static size_t const DROP_COUNT_RESET_VALUE = 1024;
	static double s_resendDelaySeconds;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	ConnectionInfo m_connectionInfo;
	NetSession const * const m_session;

//public for debugging
public:
	double m_timeLastSent;
	double m_timeLastRecv;
	int m_dropsCounted;
	int m_receivedCounted;

	//Sending
	uint16_t m_nextToSendAck;

	//Receiving
	uint16_t m_mostRecentReceivedAck;
	uint16_t m_mostRecentReceivedAcksBitfield;

	//Sending
	uint16_t m_nextToSendReliableID;

	//Confirming
	uint16_t m_nextUnconfirmedReliableID;
	uint16_t m_oldestUnconfirmedReliableID;
	uint16_t m_confirmedReliableIDs[MAX_RELIABLE_RANGE];

	//Receiving
	uint16_t m_nextUnreceivedReliableID;
	uint16_t m_oldestUnreceivedReliableID;
	uint16_t m_receivedReliableIDs[MAX_RELIABLE_RANGE];

	//Sequencing
	uint16_t m_nextToSendSequenceID[MAX_SEQUENCE_CHANNELS];
	uint16_t m_nextToReceiveReliableSequenceID[MAX_SEQUENCE_CHANNELS];

	//Nuonce number must match response nuonce
	uint32_t m_lastJoinRequestNuonce;

	//debugging information
	size_t m_lastOutgoingPackageCount;
	size_t m_lastOutgoingMessageCount;
	size_t m_lastOutgoingByteCountPacketHeader;
	size_t m_lastOutgoingByteCountMessages;

private:
	AckBundle m_bundles[MAX_ACK_BUNDLES];
	std::queue<NetMessage*> m_unsentUnreliableMessages;
	std::queue<NetMessage*> m_unsentReliableMessages;
	std::queue<NetMessage*> m_sentReliableMessages;
	NetMessage * m_sequenceChannels[MAX_SEQUENCE_CHANNELS]; //#TODO: change to channelInfos

	double m_roundTripTime;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	NetConnection( unsigned char index, sockaddr_in const & address, std::string const & guid, std::string const & username, NetSession const * const session );
	~NetConnection( );

	void AddMessage( NetMessage & message );
	void AddBundle( AckBundle const & bundle );
	void SendPacket( );
	size_t WriteSentReliables( NetPacket * packet, AckBundle * bundle, size_t * out_bytesWritten );
	size_t WriteUnsentReliables( NetPacket * packet, AckBundle * bundle, size_t * out_bytesWritten );
	size_t WriteUnsentUnreliables( NetPacket * packet, size_t * out_bytesWritten );
	void CleanUpRemainingUnreliables( );
	void ProcessMessage( NetSender const & sender, NetMessage const & message );
	void ProcessReliableSequence( NetSender const & sender, NetMessage const & message );
	void ProcessNextInSequenceChannel( NetSender const & sender, byte_t sequenceChannelID );
	void AddMessageToSequenceChannel( NetMessage const & message, byte_t sequenceChannelID );
	void ProcessUnreliableSequence( NetSender const & sender, NetMessage const & message );
	void SetPassword( size_t password );

	void UpdateLastRecvTime( );
	void MarkPacketReceived( PacketHeader const & header );
	bool MarkAckReceived( uint16_t ackID );
	void ConfirmAcksSent( uint16_t highestAck, uint16_t ackBitfield );
	void ConfirmAckBundle( uint16_t ack );
	void ConfirmReliableID( uint16_t reliableID );
	void UpdateOldestUnconfirmedReliableID( );
	void UpdateOldestUnreceivedReliableID( );
	bool MarkMessageReceived( NetMessage const & message );
	
	float GetDropRate( ) const;
	double GetRoundTripTime( ) const;
	byte_t GetIndex( ) const;
	char const * GetGUID( ) const;
	char const * GetUsername( ) const;
	size_t GetPassword( ) const;
	sockaddr_in const & GetAddress( ) const;
	NetSession const * const GetSession( ) const;
	uint16_t GetNextAck( );
	uint16_t GetNextReliableID( );
	uint16_t GetSequenceID( byte_t sequenceChannelID );

	bool IsSelf( ) const;
	bool IsTimeForHeartbeat( ) const;
	bool IsBad( ) const;
	bool IsReliableIDReceived( uint16_t reliableID ) const;
	bool IsReliableIDConfirmed( uint16_t reliableID ) const;
	bool IsMessageOld( NetMessage const * message ) const;
	bool IsHost( ) const;
	bool CanSendNewReliables( ) const;

	void SetConnectionInfo( ConnectionInfo const & connInfo );
};