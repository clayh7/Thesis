#pragma once

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class NetMessage;
class NetSender;


//-------------------------------------------------------------------------------------------------
typedef void ( MessageCallback )( NetSender const &, NetMessage const & );


//-------------------------------------------------------------------------------------------------
enum eNetMessageType : byte_t
{
	eNetMessageType_PING,
	eNetMessageType_PONG,
	eNetMessageType_JOIN_REQUEST,
	eNetMessageType_JOIN_DENY,
	eNetMessageType_JOIN_ACCEPT,
	eNetMessageType_LEAVE,
	eNetMessageType_COUNT,
	eNetMessageType_INVALID = 255,
};


//-------------------------------------------------------------------------------------------------
class NetMessageDefinition
{
//-------------------------------------------------------------------------------------------------
// Statics
//-------------------------------------------------------------------------------------------------
public:
	static byte_t const CONNECTIONLESS_CONTROL_FLAG = BIT( 0 );
	static byte_t const RELIABLE_OPTION_FLAG = BIT( 0 );
	static byte_t const SEQUENCE_OPTION_FLAG = BIT( 1 );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	eNetMessageType type;
	byte_t controlFlags;
	byte_t optionFlags;
	byte_t sequenceChannelID;
	size_t headerSize;
	MessageCallback * callback;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	void CalculateHeaderSize( );
	bool IsConnectionless( ) const;
	bool IsReliable( ) const;
	bool IsSequence( ) const;
};