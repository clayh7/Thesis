#pragma once

#include "Engine/DebugSystem/DebugLog.hpp"
#include "Engine/Net/Session/PacketChannel.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Utils/NetworkUtils.hpp"

#define NET_VERSION 2
/* Version Log
	2:  SEND_RATE = 1/60, MAX_PACKETS = 5, MTU = 1444
	1:	First version
*/

//-------------------------------------------------------------------------------------------------
class NamedProperties;
class NetConnection;
class NetMessage;
class ConnectionInfo;
class NetMessageDefinition;
class NetSender;


//---------------------------------------------------------------------------------------------
void OnPing( NetSender const &, NetMessage const & );
void OnPong( NetSender const &, NetMessage const & );
void OnJoinRequest( NetSender const &, NetMessage const & );
void OnJoinDeny( NetSender const &, NetMessage const & );
void OnJoinAccept( NetSender const &, NetMessage const & );
void OnLeave( NetSender const &, NetMessage const & );


//-------------------------------------------------------------------------------------------------
enum eNetSessionState
{
	eNetSessionState_INVALID,
	eNetSessionState_SETUP,
	eNetSessionState_DISCONNECTED,
	eNetSessionState_HOSTING,
	eNetSessionState_CONNECTED,
	eNetSessionState_JOINING,
};


//-------------------------------------------------------------------------------------------------
enum eNetSessionError
{
	eNetSessionError_NONE,
	eNetSessionError_SOCKET_CREATION_FAILED,
	eNetSessionError_JOIN_HOST_TIMEOUT,
	eNetSessionError_JOIN_DENIED_WRONG_VERSION,
	eNetSessionError_JOIN_DENIED_NOT_ACCEPTING_NEW_CONNECTIONS,
	eNetSessionError_JOIN_DENIED_NOT_HOST,
	eNetSessionError_JOIN_DENIED_FULL,
	eNetSessionError_JOIN_DENIED_GUID_IN_USE,
	eNetSessionError_HOST_CREATION_FAILED,
	eNetSessionError_HOST_DISCONNECTED,
	eNetSessionError_COUNT,
};


//-------------------------------------------------------------------------------------------------
class JoinRequest
{
public:
	uint64_t version;
	uint32_t nuonce;
	char * username;
	size_t password;
};


//-------------------------------------------------------------------------------------------------
class NetSession
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static int const PORT_RANGE = 12; //default
	static int const MAX_CONNECTIONS = 255; //largest number possible of Byte - 1
	static int const MAX_DEFINITIONS = 256; //largest number possible of Byte
	static float const SEND_RATE; //seconds per packets
	static uint8_t const HOST_INDEX = 0;

public:
	static byte_t const INVALID_INDEX = 255; //last connection index is the invalid index
	static int const MAX_PACKET_SEND_AMOUNT_PER_CONNECTION; //max number of packets sent during ProcessOutgoingPackets();
	static float const HEARTBEAT_INTERVAL_SECONDS;
	static float const BAD_CONNECTION_INTERVAL_SECONDS;
	static float const DISCONNECT_INTERVAL_SECONDS;
	static char const * PREPARE_PACKET_EVENT;
	static char const * ON_CONNECTION_JOIN_EVENT;
	static char const * ON_CONNECTION_LEAVE_EVENT;
	static char const * ON_GAME_JOIN_VALIDATION_EVENT;
	static char const * ON_JOIN_DENY_EVENT;
	static DebugLog m_NetworkTrafficActivity;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	PacketChannel m_channel;
	NetConnection * m_connections[MAX_CONNECTIONS];
	NetConnection * m_self;
	NetConnection * m_host;
	eNetSessionState m_state;
	NetMessageDefinition * m_messageDefinitions[MAX_DEFINITIONS];
	byte_t m_definitionCount;
	bool m_connectionTimeouts;

	uint16_t m_netVersion;
	uint32_t m_definitionHash;
	uint16_t m_gameVersion;

public:
	int m_invalidPacketCount;
	int m_invalidMessageCount;
	eNetSessionError m_lastError;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	NetSession( uint16_t gameVersion = 0U );
	~NetSession( );

	void OnUpdate( NamedProperties & );
	void ProcessIncomingPackets( );
	void ProcessOutgoingPackets( );
	void CheckForDisconnect( );

	void RegisterMessage( eNetMessageType const & type, MessageCallback * cb, byte_t const & setTypeFlags = 0, byte_t const & setOptionFlags = 0, byte_t const & setChannel = 0 );
	bool Start( unsigned int port, unsigned int range = PORT_RANGE );
	void Stop( );
	void Host( char const * username, size_t password = 0 );
	void Join( sockaddr_in const & hostAddress, char const * username, size_t password = 0 );
	void Leave( );
	NetConnection * CreateConnection( ConnectionInfo const & connInfo ) const;
	NetConnection * CreateConnection( byte_t index, sockaddr_in const & addr, std::string const & guid, std::string const & username ) const;
	void DisconnectOtherClientConnections( );
	void Connect( NetConnection * connection );
	void Disconnect( NetConnection ** connection );
	void AddMessageToAllClients( NetMessage & message );
	void SendDirect( sockaddr_in const & address, NetMessage & message ) const;
	void SendDeny( sockaddr_in const & address, eNetSessionError const & reason, uint32_t nuonce ) const;
	void SendAccept( NetConnection * connInfo, uint32_t nuonce ) const;
	void SendPacket( sockaddr_in const & address, byte_t const * data, size_t dataSize ) const;
	void ProcessPacket( NetPacket & packet );
	void ReadMessage( NetPacket const & packet, NetMessage * out_message );
	void PrintError( eNetSessionError const & error );

	void ChangeState( eNetSessionState const & newState );
	void EnterState( eNetSessionState const & state );
	void LeaveState( eNetSessionState const & state );
	void UpdateState( eNetSessionState const & state );

	NetConnection * GetNetConnection( sockaddr_in const & address ) const;
	NetConnection * GetNetConnection( uint8_t netIndex );
	PacketChannel const & GetPacketChannel( ) const;
	UDPSock const & GetSocket( ) const;
	char const * GetAddressString( ) const;
	NetMessageDefinition const * GetDefinition( eNetMessageType const & type ) const;
	NetConnection * GetSelf( ) const;
	NetConnection * GetHost( ) const;
	eNetSessionState GetState( ) const;
	float GetSimDropRate( ) const;
	Range<double> const GetLatency( ) const;
	uint64_t GetNetSessionVersion( ) const;
	uint32_t GetNuonce( ) const;
	byte_t GetNextFreeIndex( ) const;
	bool IsConnected( ) const;
	bool IsValidPacket( NetPacket const & packet, size_t packetSize ) const;
	bool IsValidMessage( NetSender const & senderInfo, NetMessage const & message ) const;
	bool IsValidConnectionIndex( byte_t index ) const;
	bool IsDuplicateGUID( std::string const & check ) const;
	bool IsHost( ) const;

	void SetDropRate( float dropRate );
	void SetLatency( Range<double> latency );
	bool ToggleTimeouts( );
};