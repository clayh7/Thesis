#include "Engine/Net/Session/NetSession.hpp"

#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/Net/NetworkSystem.hpp"
#include "Engine/Net/Session/NetMessage.hpp"
#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/Net/Session/NetConnection.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC float const NetSession::SEND_RATE = 1.f / 60.f; //60 Hz

STATIC int const NetSession::MAX_PACKET_SEND_AMOUNT_PER_CONNECTION = 5;
STATIC float const NetSession::HEARTBEAT_INTERVAL_SECONDS = 1.f;
STATIC float const NetSession::BAD_CONNECTION_INTERVAL_SECONDS = 5.f;
STATIC float const NetSession::DISCONNECT_INTERVAL_SECONDS = 15.f;
STATIC char const * NetSession::PREPARE_PACKET_EVENT = "PreparePacketEvent";
STATIC char const * NetSession::ON_CONNECTION_JOIN_EVENT = "ConnectionJoinEvent";
STATIC char const * NetSession::ON_CONNECTION_LEAVE_EVENT = "ConnectionLeaveEvent";
STATIC char const * NetSession::ON_GAME_JOIN_VALIDATION_EVENT = "GameJoinValidation";
STATIC char const * NetSession::ON_JOIN_DENY_EVENT = "JoinDenyEvent";
STATIC DebugLog NetSession::m_NetworkTrafficActivity = DebugLog( "Data/Logs/NetworkTraffic.txt" );


//-------------------------------------------------------------------------------------------------
class Ping
{
public:
	char * message;
};


//-------------------------------------------------------------------------------------------------
void OnPing( NetSender const & sender, NetMessage const & message )
{
	char * sent = nullptr;
	message.ReadString( &(sent) );
	sent = sent == nullptr ? "[null]" : sent;
	sockaddr_in addr = sender.fromAddress;
	g_ConsoleSystem->AddLog( Stringf( "Received Ping from %s: %s", StringFromSockAddr(&addr), sent ), Console::GOOD );

	NetMessage pong( eNetMessageType_PONG );
	sender.session->SendDirect( addr, pong );
}


//-------------------------------------------------------------------------------------------------
class Pong
{
	//Nothing
};


//-------------------------------------------------------------------------------------------------
void OnPong( NetSender const & sender, NetMessage const & )
{
	sockaddr_in addr = sender.fromAddress;
	g_ConsoleSystem->AddLog( Stringf( "Received Pong from %s", StringFromSockAddr( &addr ) ), Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
void OnJoinRequest( NetSender const & sender, NetMessage const & message )
{
	g_ConsoleSystem->AddLog( Stringf( "Join Request Received: %s", StringFromSockAddr( &sender.fromAddress ) ), Console::REMOTE );
	NetSession * currentSession = sender.session;

	JoinRequest request;
	message.Read<uint64_t>( &( request.version ) );
	message.Read<uint32_t>( &( request.nuonce ) );
	message.ReadString( &( request.username ) );
	message.Read<size_t>( &( request.password ) );

	if( request.version != currentSession->GetNetSessionVersion( ) )
	{
		currentSession->SendDeny( sender.fromAddress, eNetSessionError_JOIN_DENIED_WRONG_VERSION, request.nuonce );
	}
	else if( !currentSession->IsHost() )
	{
		currentSession->SendDeny( sender.fromAddress, eNetSessionError_JOIN_DENIED_NOT_HOST, request.nuonce );
	}
	//#TODO: implement listening
	else if( false )
	{
		currentSession->SendDeny( sender.fromAddress, eNetSessionError_JOIN_DENIED_NOT_ACCEPTING_NEW_CONNECTIONS, request.nuonce );
	}
	//TODO: implement max capacity
	else if( false )
	{
		currentSession->SendDeny( sender.fromAddress, eNetSessionError_JOIN_DENIED_FULL, request.nuonce );
	}
	//Socket Address == GUID
	else if( currentSession->IsDuplicateGUID( StringFromSockAddr( &sender.fromAddress ) ) )
	{
		currentSession->SendDeny( sender.fromAddress, eNetSessionError_JOIN_DENIED_GUID_IN_USE, request.nuonce );
	}
	else
	{
		NamedProperties joinEvent;
		joinEvent.Set( "request", &request );
		joinEvent.Set( "error", eNetSessionError_NONE );
		joinEvent.Set( "canJoin", true );
		EventSystem::TriggerEvent( NetSession::ON_GAME_JOIN_VALIDATION_EVENT, joinEvent );

		//Find out if join was successful
		bool success = false;
		joinEvent.Get( "canJoin", success );

		if( success )
		{
			//Create new connection
			byte_t index = currentSession->GetNextFreeIndex( );
			ConnectionInfo newConnectionInfo( index, sender.fromAddress, StringFromSockAddr( &sender.fromAddress ), request.username );
			NetConnection * newConnection = currentSession->CreateConnection( newConnectionInfo );
			newConnection->SetPassword( request.password );

			//Send Acceptance
			currentSession->SendAccept( newConnection, request.nuonce );

			//Conenct after sending Accept because we want the send accept message written first
			currentSession->Connect( newConnection );
		}
		else
		{
			eNetSessionError reportedError;
			joinEvent.Get( "error", reportedError );
			currentSession->SendDeny( sender.fromAddress, reportedError, request.nuonce );
		}
	}
}


//-------------------------------------------------------------------------------------------------
class JoinDeny
{
public:
	uint32_t nounce;
	uint8_t reason;
};


//-------------------------------------------------------------------------------------------------
void OnJoinDeny( NetSender const & sender, NetMessage const & message )
{
	EventSystem::TriggerEvent( NetSession::ON_JOIN_DENY_EVENT );
	g_ConsoleSystem->AddLog( Stringf( "Join Deny Received: %s", StringFromSockAddr( &sender.fromAddress ) ), Console::REMOTE );
	uint32_t nuonce;
	message.Read<uint32_t>( &nuonce );

	uint8_t reason;
	message.Read<uint8_t>( &reason );

	NetConnection * self = sender.session->GetSelf( );
	if( sender.session->GetState( ) != eNetSessionState_JOINING )
	{
		return;
	}
	else if( self->m_lastJoinRequestNuonce != nuonce )
	{
		return;
	}

	sender.session->m_lastError = (eNetSessionError) reason;
	sender.session->ChangeState( eNetSessionState_DISCONNECTED );
}


//-------------------------------------------------------------------------------------------------
class JoinAccept
{
public:
	uint32_t nuonce;
	ConnectionInfo hostInfo;
	ConnectionInfo joineeInfo;
};


//-------------------------------------------------------------------------------------------------
void OnJoinAccept( NetSender const & sender, NetMessage const & message )
{
	g_ConsoleSystem->AddLog( Stringf( "Join Accept Received: %s", StringFromSockAddr( &sender.fromAddress ) ), Console::REMOTE );
	NetSession * currentSession = sender.session;
	
	//Must be in joining state
	if( currentSession->GetState( ) != eNetSessionState_JOINING )
	{
		return;
	}
	
	//Must have connection to self
	NetConnection * currentConnection = currentSession->GetSelf( );
	if( !currentConnection )
	{
		return;
	}

	//Read accept nuonce
	uint32_t nuonce;
	message.Read<uint32_t>( &( nuonce ) );
	
	if( currentConnection->m_lastJoinRequestNuonce != nuonce )
	{
		return;
	}
	
	//Read host info
	byte_t index;
	message.Read<byte_t>( &index );
	char * hostUsername;
	message.ReadString( &( hostUsername ) );
	ConnectionInfo hostInfo( index, sender.fromAddress, StringFromSockAddr( &sender.fromAddress ), hostUsername );
	NetConnection * host = currentSession->GetHost( );
	host->SetConnectionInfo( hostInfo );

	//Establish Connection
	currentSession->Connect( host );

	//Read my info
	message.Read<byte_t>( &index );
	ConnectionInfo joineeInfo = ConnectionInfo( index, currentConnection->GetAddress( ), currentConnection->GetGUID( ), currentConnection->GetUsername() );
	currentConnection->SetConnectionInfo( joineeInfo );

	//Establish Connection
	currentSession->Connect( currentConnection );

	currentSession->ChangeState( eNetSessionState_CONNECTED );
}


//-------------------------------------------------------------------------------------------------
void OnLeave( NetSender const & sender, NetMessage const & )
{
	g_ConsoleSystem->AddLog( Stringf( "Leave Received: %s", StringFromSockAddr( &sender.fromAddress ) ), Console::REMOTE );
	NetConnection * connection = sender.session->GetNetConnection( sender.connection->GetIndex( ) );

	//Host left, so I should leave
	if( connection == sender.session->GetHost( ) )
	{
		sender.session->ChangeState( eNetSessionState_DISCONNECTED );
	}

	//Someone else left
	else if( connection )
	{
		sender.session->Disconnect( &connection );
	}
}


//-------------------------------------------------------------------------------------------------
NetSession::NetSession( uint16_t gameVersion /*= 0U*/ )
	: m_channel( )
	, m_self( nullptr )
	, m_host( nullptr )
	, m_state( eNetSessionState_INVALID )
	, m_definitionCount( 0 )
	, m_connectionTimeouts( true )
	, m_invalidPacketCount( 0 )
	, m_invalidMessageCount( 0 )
	, m_lastError( eNetSessionError_NONE )
	, m_netVersion( NET_VERSION )
	, m_definitionHash( 0U )
	, m_gameVersion( gameVersion )
{
	//Clear out all connections
	for( size_t index = 0; index < MAX_CONNECTIONS; ++index )
	{
		m_connections[index] = nullptr;
	}

	//Clear out all messages
	for( size_t defIndex = 0; defIndex < MAX_DEFINITIONS; ++defIndex )
	{
		m_messageDefinitions[defIndex] = nullptr;
	}

	EventSystem::RegisterEvent( ENGINE_UPDATE_EVENT, this, &NetSession::OnUpdate );

	//Registering Core Message Types
	byte_t controlFlags, optionFlags;

	//Connectionless, Unreliable
	controlFlags = NetMessageDefinition::CONNECTIONLESS_CONTROL_FLAG;
	optionFlags = 0;
	RegisterMessage( eNetMessageType_PING, OnPing, controlFlags, optionFlags );
	RegisterMessage( eNetMessageType_PONG, OnPong, controlFlags, optionFlags );

	//Connectionless, Reliable
	controlFlags = NetMessageDefinition::CONNECTIONLESS_CONTROL_FLAG;
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG;
	RegisterMessage( eNetMessageType_JOIN_REQUEST, OnJoinRequest, controlFlags, optionFlags );

	//Connection, Unreliable
	controlFlags = NetMessageDefinition::CONNECTIONLESS_CONTROL_FLAG;
	optionFlags = 0;
	RegisterMessage( eNetMessageType_JOIN_DENY, OnJoinDeny, controlFlags, optionFlags );
	RegisterMessage( eNetMessageType_LEAVE, OnLeave, controlFlags, optionFlags );

	//Connection, Reliable
	controlFlags = NetMessageDefinition::CONNECTIONLESS_CONTROL_FLAG;
	optionFlags = NetMessageDefinition::RELIABLE_OPTION_FLAG;
	RegisterMessage( eNetMessageType_JOIN_ACCEPT, OnJoinAccept, controlFlags, optionFlags );
}


//-------------------------------------------------------------------------------------------------
NetSession::~NetSession( )
{
	EventSystem::Unregister( this );


	//Delete all registered messages
	//I had to keep track of my number of message definitions, but not the number of connections I had
	//message definitions was breaking when deleting a nullptr, weird.
	for( size_t defIndex = 0; defIndex < m_definitionCount; ++defIndex )
	{
		delete m_messageDefinitions[defIndex];
		m_messageDefinitions[defIndex] = nullptr;
	}

	Disconnect( &m_self );
	DisconnectOtherClientConnections( );
	Disconnect( &m_host );

	m_channel.Unbind( );
}


//-------------------------------------------------------------------------------------------------
void NetSession::OnUpdate( NamedProperties & )
{
	UpdateState( m_state );
}


//-------------------------------------------------------------------------------------------------
void NetSession::ProcessIncomingPackets( )
{
	m_channel.RecvPackets( this );
}


//-------------------------------------------------------------------------------------------------
void NetSession::ProcessOutgoingPackets( )
{
	static float timeSinceLastUpdate = 0.f;
	timeSinceLastUpdate += Time::DELTA_SECONDS;
	if( timeSinceLastUpdate >= SEND_RATE )
	{
		for(size_t index = 0; index < MAX_CONNECTIONS; ++index )
		{
			if( !m_connections[index] )
			{
				continue;
			}
			NamedProperties netEvent;
			netEvent.Set( "connection", m_connections[index] );
			EventSystem::TriggerEvent( PREPARE_PACKET_EVENT, netEvent );
			m_connections[index]->SendPacket( );
		}

		//I only want to process packets at most once per frame
		while( timeSinceLastUpdate >= SEND_RATE )
		{
			timeSinceLastUpdate -= SEND_RATE;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::CheckForDisconnect( )
{
	if( m_host )
	{
		float timeElapsed = (float) ( Time::TOTAL_SECONDS - m_host->m_timeLastRecv );
		if( timeElapsed > DISCONNECT_INTERVAL_SECONDS && m_connectionTimeouts )
		{
			g_ConsoleSystem->AddLog( "Connection to host timed out", Console::BAD );
			ChangeState( eNetSessionState_DISCONNECTED );
		}
	}

	for( size_t index = 0; index < MAX_CONNECTIONS; ++index )
	{
		if( !m_connections[index] )
		{
			continue;
		}
		
		float timeElapsed = ( float )( Time::TOTAL_SECONDS - m_connections[index]->m_timeLastRecv );
		if( timeElapsed > DISCONNECT_INTERVAL_SECONDS && m_connectionTimeouts )
		{
			g_ConsoleSystem->AddLog( Stringf( "Connection %d timed out", index ), Console::BAD );
			Disconnect( &m_connections[index] );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::RegisterMessage( eNetMessageType const & type, MessageCallback * cb, byte_t const & setTypeFlags /*= 0*/, byte_t const & setOptionFlags /*= 0*/, byte_t const & setChannel /*= 0 */ )
{
	if( GetState( ) != eNetSessionState_INVALID )
	{
		g_ConsoleSystem->AddLog( "Can only register messages before starting", Console::BAD );
		return;
	}

	++m_definitionCount;
	NetMessageDefinition * def = new NetMessageDefinition();
	def->type = type;
	def->controlFlags = setTypeFlags;
	def->optionFlags = setOptionFlags;
	def->sequenceChannelID = setChannel;
	def->CalculateHeaderSize( );
	def->callback = cb;
	m_messageDefinitions[type] = def;
}


//-------------------------------------------------------------------------------------------------
bool NetSession::Start( unsigned int port, unsigned int range /*= PORT_RANGE*/ )
{
	if( GetState( ) != eNetSessionState_INVALID )
	{
		g_ConsoleSystem->AddLog( "Net Session already started", Console::BAD );
		return false;
	}

	ChangeState( eNetSessionState_SETUP );
	m_channel.Bind( NetworkUtils::GetLocalHostName( ), port, range );

	//If socket is connected
	if( IsConnected( ) )
	{
		//Set our state to ready, but no host connection
		ChangeState( eNetSessionState_DISCONNECTED );
		g_ConsoleSystem->AddLog( Stringf( "Net Session: %s", GetAddressString( ) ), Console::GOOD );

		//#TODO: Unique identifier for the definitions registered
		m_definitionHash = m_definitionCount;
		return true;
	}
	else
	{
		m_lastError = eNetSessionError_SOCKET_CREATION_FAILED;
		ChangeState( eNetSessionState_INVALID );
		g_ConsoleSystem->AddLog( "Start Net Session failed", Console::BAD );
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::Stop( )
{
	if( GetState( ) == eNetSessionState_INVALID )
	{
		g_ConsoleSystem->AddLog( "Net Session not currently running", Console::BAD );
		return;
	}
	else if( GetState( ) != eNetSessionState_DISCONNECTED )
	{
		g_ConsoleSystem->AddLog( "Must be disconnected to stop", Console::BAD );
		return;
	}

	m_channel.Unbind( );
	ChangeState( eNetSessionState_INVALID );
	g_ConsoleSystem->AddLog( "Successfully stopped Net Session", Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
void NetSession::Host( char const * username, size_t password /*= 0*/ )
{
	if( GetState( ) != eNetSessionState_DISCONNECTED )
	{
		g_ConsoleSystem->AddLog( "Must be disconnected to host", Console::BAD );
		return;
	}

	//Make a connection for yourself at conn_index 0
	ChangeState( eNetSessionState_HOSTING );
	m_host = CreateConnection( HOST_INDEX, m_channel.GetAddress( ), GetAddressString(), username );
	if( m_host )
	{
		m_host->SetPassword( password );
		ChangeState( eNetSessionState_CONNECTED );
		g_ConsoleSystem->AddLog( Stringf( "Hosting: %s", GetAddressString( ) ), Console::GOOD );
		m_self = m_host;
		Connect( m_host );
	}
	else
	{
		ChangeState( eNetSessionState_DISCONNECTED );
		m_lastError = eNetSessionError_HOST_CREATION_FAILED;
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::Join( sockaddr_in const & hostAddress, char const * username, size_t password /*= 0*/ )
{
	if( GetState( ) != eNetSessionState_DISCONNECTED )
	{
		g_ConsoleSystem->AddLog( "Must be disconnected to join a host", Console::BAD );
		return;
	}

	g_ConsoleSystem->AddLog( Stringf( "Joining: %s", StringFromSockAddr( &hostAddress ) ), Console::GOOD );
	m_host = CreateConnection( HOST_INDEX, hostAddress, StringFromSockAddr( &hostAddress ), " " );
	if( m_host )
	{
		ChangeState( eNetSessionState_JOINING );

		//Make my connection
		m_self = CreateConnection( INVALID_INDEX, m_channel.GetAddress( ), GetAddressString(), username );
		m_self->SetPassword( password );
		m_self->m_lastJoinRequestNuonce = GetNuonce( );

		NetMessage request( eNetMessageType_JOIN_REQUEST );
		request.Write<uint64_t>( GetNetSessionVersion( ) );
		request.Write<uint32_t>( m_self->m_lastJoinRequestNuonce );
		request.WriteString( username );
		request.Write<size_t>( password );
		m_host->AddMessage( request );
		m_host->SendPacket( );
	}
	else
	{
		ChangeState( eNetSessionState_DISCONNECTED );
		m_lastError = eNetSessionError_HOST_CREATION_FAILED;
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::Leave( )
{
	if( GetState( ) != eNetSessionState_CONNECTED )
	{
		g_ConsoleSystem->AddLog( "Must be connected to a host to leave", Console::BAD );
		return;
	}

	//send everyone else a LEAVE message
	NetMessage leave( eNetMessageType_LEAVE, GetSelf( )->GetIndex( ) );
	for( uint8_t connIndex = 0; connIndex < MAX_CONNECTIONS; ++connIndex )
	{
		NetConnection * conn = GetNetConnection( connIndex );
		if( conn && conn != m_self )
		{
			conn->AddMessage( leave );
			conn->SendPacket( );
		}
	}

	//Disconnect self & others & host
	ChangeState( eNetSessionState_DISCONNECTED );

	g_ConsoleSystem->AddLog( "Successful leave", Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
NetConnection * NetSession::CreateConnection( ConnectionInfo const & connInfo ) const
{
	return CreateConnection( connInfo.m_index, connInfo.m_address, connInfo.m_guid, connInfo.m_username );
}


//-------------------------------------------------------------------------------------------------
NetConnection * NetSession::CreateConnection( byte_t index, sockaddr_in const & address, std::string const & guid, std::string const & username ) const
{
	if( GetState( ) == eNetSessionState_INVALID )
	{
		g_ConsoleSystem->AddLog( "Net Session not currently running", Console::BAD );
		return nullptr;
	}

	if( m_connections[index] )
	{
		g_ConsoleSystem->AddLog( "Connection index already exists", Console::BAD );
		return nullptr;
	}

	if( IsDuplicateGUID( guid ) )
	{
		g_ConsoleSystem->AddLog( "GUID already exists", Console::BAD );
		return nullptr;
	}

	g_ConsoleSystem->AddLog( Stringf( "Connection %u created", index ), Console::GOOD );
	return new NetConnection( index, address, guid, username, this );
}


//-------------------------------------------------------------------------------------------------
void NetSession::DisconnectOtherClientConnections( )
{
	for( size_t index = 0; index < MAX_CONNECTIONS; ++index )
	{
		if( m_connections[index] == m_host )
		{
			//Nothing
		}
		else if( m_connections[index] == m_self )
		{
			//Nothing
		}
		else
		{
			Disconnect( &m_connections[index] );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::Connect( NetConnection * connection )
{
	if( m_connections[connection->GetIndex( )] )
	{
		g_ConsoleSystem->AddLog( "Connection already exists", Console::BAD );
		return;
	}

	//Set Connection
	m_connections[connection->GetIndex()] = connection;

	//Trigger Join Event
	NamedProperties netEvent;
	netEvent.Set( "connection", connection );
	EventSystem::TriggerEvent( ON_CONNECTION_JOIN_EVENT, netEvent );

	g_ConsoleSystem->AddLog( Stringf( "Connection %u established", connection->GetIndex() ), Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
void NetSession::Disconnect( NetConnection ** connection )
{
	//We can call this on a nullptr, no big deal
	if( ( *connection ) == nullptr )
	{
		return;
	}

	//If you're host, clear the host and trigger leave
	if( m_self == m_host && ( *connection ) == m_host )
	{
		//Trigger Leave Event
		NamedProperties netEvent;
		netEvent.Set("connection", *connection );
		EventSystem::TriggerEvent( ON_CONNECTION_LEAVE_EVENT, netEvent );
		m_host = nullptr;
	}

	byte_t index = ( *connection )->GetIndex( );
	//Must have been temp, delete them
	if( index == INVALID_INDEX )
	{
		delete *connection;
		*connection = nullptr;
		return;
	}
	//You don't exist, this is a problem... but we'll still clear you out anyways
	//Aaand the 'temp' host will call this, this is fine
	else if( m_connections[index] == nullptr )
	{
		delete *connection;
		*connection = nullptr;
		g_ConsoleSystem->AddLog( "Connection not established", Console::BAD );
		return;
	}

	//Trigger Leave Event
	NamedProperties netEvent;
	netEvent.Set( "connection", *connection );
	EventSystem::TriggerEvent( ON_CONNECTION_LEAVE_EVENT, netEvent );

	//Clear connection
	delete m_connections[index];
	m_connections[index] = nullptr;
	*connection = nullptr;

	g_ConsoleSystem->AddLog( Stringf( "Connection %u disconnected", index ), Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
void NetSession::AddMessageToAllClients( NetMessage & message )
{
	for( byte_t connIndex = 0; connIndex < MAX_CONNECTIONS; ++connIndex )
	{
		if( m_connections[connIndex] == nullptr )
		{
			continue;
		}

		m_connections[connIndex]->AddMessage( message );
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::SendDirect( sockaddr_in const & address, NetMessage & message ) const
{
	if( GetState( ) == eNetSessionState_INVALID )
	{
		g_ConsoleSystem->AddLog( "Start session first", Console::BAD );
		return;
	}

	NetMessageDefinition const * definition = GetDefinition( message.GetNetMessageType( ) );

	if( definition->IsReliable( ) )
	{
		g_ConsoleSystem->AddLog( "Cannot send direct a message that is reliable", Console::BAD );
		return;
	}

	NetConnection conn( INVALID_INDEX, address, "", "", this);
	conn.AddMessage( message );
	conn.SendPacket( );
}


//-------------------------------------------------------------------------------------------------
void NetSession::SendDeny( sockaddr_in const & address, eNetSessionError const & reason, uint32_t nuonce ) const
{
	g_ConsoleSystem->AddLog( Stringf( "Deny Sent: %s", StringFromSockAddr( &address ) ), Console::REMOTE );
	NetMessage deny( eNetMessageType_JOIN_DENY );
	deny.Write<uint32_t>( nuonce );
	deny.Write<uint8_t>( (uint8_t) reason );
	SendDirect( address, deny );
}


//-------------------------------------------------------------------------------------------------
void NetSession::SendAccept( NetConnection * connection, uint32_t nuonce ) const
{
	g_ConsoleSystem->AddLog( Stringf( "Accept Sent: %s", StringFromSockAddr( &connection->GetAddress() ) ), Console::REMOTE );
	NetMessage accept( eNetMessageType_JOIN_ACCEPT );

	accept.Write<uint32_t>( nuonce );
	accept.Write<byte_t>( m_host->GetIndex( ) );
	accept.WriteString( m_host->GetUsername( ) );
	accept.Write<byte_t>( connection->GetIndex() );

	connection->AddMessage( accept );
	//Do not need to immediately send a packet, it's connected, so it will automatically send OnUpdate()
}


//-------------------------------------------------------------------------------------------------
void NetSession::SendPacket( sockaddr_in const & address, byte_t const * data, size_t dataSize ) const
{
	m_channel.SendPackets( address, data, dataSize );
}


//-------------------------------------------------------------------------------------------------
void NetSession::ProcessPacket( NetPacket & packet )
{
	PacketHeader header;
	packet.ReadHeader( &header );

	//Connection received packet
	packet.m_senderInfo.connection = GetNetConnection( packet.m_senderInfo.fromAddress );

	//Still need to make packets with no contents
	if( header.messageCount == 0 )
	{
		if( packet.m_senderInfo.connection != nullptr )
		{
			packet.m_senderInfo.connection->MarkPacketReceived( header );
		}
	}

	for( size_t messageIndex = 0; messageIndex < header.messageCount; ++messageIndex )
	{
		//Read Message
		NetMessage message;
		ReadMessage( packet, &message );

		//Set Ack ID from packet (used in unreliable sequenced messages)
		//message.m_ackID = header.packetAck;

		//Validate and process
		if( packet.m_senderInfo.connection != nullptr )
		{
			if( IsValidMessage( packet.m_senderInfo, message ) )
			{
				packet.m_senderInfo.connection->ProcessMessage( packet.m_senderInfo, message );
				packet.m_senderInfo.connection->MarkPacketReceived( header );
			}
			else
			{
				++m_invalidMessageCount;
				continue;
			}
		}
		else
		{
			if( !message.m_definition->IsConnectionless( ) )
			{
				++m_invalidMessageCount;
				continue;
			}

			message.Process( packet.m_senderInfo );
			
			//Check again and mark received
			if( message.m_definition->IsReliable( ) )
			{
				packet.m_senderInfo.connection = GetNetConnection( packet.m_senderInfo.fromAddress );
			}
			if( packet.m_senderInfo.connection != nullptr )
			{
				packet.m_senderInfo.connection->MarkPacketReceived( header );
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::ReadMessage( NetPacket const & packet, NetMessage * out_message )
{
	//Read Message Size
	uint16_t messageSize;
	packet.Read( &messageSize );

	//Read Type
	packet.Read( &( out_message->m_type ) );
	messageSize -= 1; //One less because we just read a byte

	out_message->m_definition = GetDefinition( (eNetMessageType) out_message->m_type );

	//Definition does not exist on session
	if( !out_message->m_definition )
	{
		++m_invalidMessageCount;
		packet.Advance( messageSize );
		return;
	}

	//Read Index
	if( !out_message->m_definition->IsConnectionless( ) )
	{
		packet.Read( &( out_message->m_senderIndex ) );
		messageSize -= 1; //One less because we just read a byte
	}

	//Read ReliableID
	if( out_message->m_definition->IsReliable( ) )
	{
		packet.Read( &( out_message->m_reliableID ) );
		messageSize -= 2; //Two less because we just read 2 bytes
	}

	//Read SequenceID
	if( out_message->m_definition->IsReliable( ) && out_message->m_definition->IsSequence( ) )
	{
		packet.Read( &( out_message->m_sequenceID ) );
		messageSize -= 2; //Two less because we just read 2 bytes
	}

	//Copy Message
	out_message->WriteForward( packet.GetHead( ), messageSize );
	out_message->Rewind( );

	//Finished with message
	packet.Advance( messageSize );
}


//-------------------------------------------------------------------------------------------------
void NetSession::PrintError( eNetSessionError const & error )
{
	switch( error )
	{
	case eNetSessionError_JOIN_HOST_TIMEOUT:
		g_ConsoleSystem->AddLog( "Join host timed out", Console::BAD );
		break;
	case eNetSessionError_JOIN_DENIED_WRONG_VERSION:
		g_ConsoleSystem->AddLog( "Join denied: Wrong version", Console::BAD );
		break;
	case eNetSessionError_JOIN_DENIED_NOT_ACCEPTING_NEW_CONNECTIONS:
		g_ConsoleSystem->AddLog( "Join denied: Not accepting new connections", Console::BAD );
		break;
	case eNetSessionError_JOIN_DENIED_NOT_HOST:
		g_ConsoleSystem->AddLog( "Join denied: Not host", Console::BAD );
		break;
	case eNetSessionError_JOIN_DENIED_FULL:
		g_ConsoleSystem->AddLog( "Join denied: Host full", Console::BAD );
		break;
	case eNetSessionError_JOIN_DENIED_GUID_IN_USE:
		g_ConsoleSystem->AddLog( "Join denied: GUID already in use", Console::BAD );
		break;
	default:
		g_ConsoleSystem->AddLog( "Unknown Error", Console::BAD );
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::ChangeState( eNetSessionState const & newState )
{
	//Never change to the same state
	if( m_state == newState )
	{
		ERROR_AND_DIE( "Net Session State change to the same state" );
	}

	LeaveState( m_state );
	m_state = newState;
	EnterState( m_state );
}


//-------------------------------------------------------------------------------------------------
void NetSession::EnterState( eNetSessionState const & state )
{
	switch( state )
	{
	case eNetSessionState_DISCONNECTED:
		Disconnect( &m_self );
		DisconnectOtherClientConnections( );
		Disconnect( &m_host );
		break;
	case eNetSessionState_JOINING:
		break;
	default:
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::LeaveState( eNetSessionState const & state )
{
	switch( state )
	{
	case eNetSessionState_INVALID:
		break;
	default:
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void NetSession::UpdateState( eNetSessionState const & state )
{
	g_ProfilerSystem->StartSample( "NET UPDATE" );
	switch( state )
	{
	case eNetSessionState_INVALID:
		break;
	case eNetSessionState_DISCONNECTED:
		ProcessIncomingPackets( );
		ProcessOutgoingPackets( );
		break;
	default:
		ProcessIncomingPackets( );
		ProcessOutgoingPackets( );
		CheckForDisconnect( );
		break;
	}
	m_NetworkTrafficActivity.WriteToFile( );
	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
NetConnection * NetSession::GetNetConnection( sockaddr_in const & address ) const
{
	for( size_t index = 0; index < MAX_CONNECTIONS; ++index )
	{
		if( !m_connections[index] )
		{
			continue;
		}
		if( IsEqual( m_connections[index]->GetAddress( ), address ) )
		{
			return m_connections[index];
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
NetConnection * NetSession::GetNetConnection( uint8_t netIndex )
{
	return m_connections[netIndex];
}


//-------------------------------------------------------------------------------------------------
PacketChannel const & NetSession::GetPacketChannel( ) const
{
	return m_channel;
}


//-------------------------------------------------------------------------------------------------
UDPSock const & NetSession::GetSocket( ) const
{
	return m_channel;
}


//-------------------------------------------------------------------------------------------------
char const * NetSession::GetAddressString( ) const
{
	return m_channel.GetAddressString( );
}


//-------------------------------------------------------------------------------------------------
NetMessageDefinition const * NetSession::GetDefinition( eNetMessageType const & type ) const
{
	return m_messageDefinitions[type];
}


//-------------------------------------------------------------------------------------------------
NetConnection * NetSession::GetSelf( ) const
{
	return m_self;
}


//-------------------------------------------------------------------------------------------------
NetConnection * NetSession::GetHost( ) const
{
	return m_host;
}


//-------------------------------------------------------------------------------------------------
eNetSessionState NetSession::GetState( ) const
{
	return m_state;
}


//-------------------------------------------------------------------------------------------------
float NetSession::GetSimDropRate( ) const
{
	return m_channel.m_dropRate;
}


//-------------------------------------------------------------------------------------------------
Range<double> const NetSession::GetLatency( ) const
{
	return m_channel.m_latency;
}


//-------------------------------------------------------------------------------------------------
uint64_t NetSession::GetNetSessionVersion( ) const
{
	uint64_t version = 0;
	version |= m_netVersion;

	version = ( version << 16 );
	version |= m_gameVersion;

	version = ( version << 32 );
	version |= m_definitionHash;
	return version;
}


//-------------------------------------------------------------------------------------------------
uint32_t NetSession::GetNuonce( ) const
{
	static uint32_t nuonce;
	nuonce += (uint8_t) ( RandomFloatZeroToOne( ) * RandomInt( 256 ) );
	return nuonce;
}


//-------------------------------------------------------------------------------------------------
byte_t NetSession::GetNextFreeIndex( ) const
{
	byte_t check = 0;
	while( m_connections[check] )
	{
		++check;
		if( check == INVALID_INDEX )
		{
			return INVALID_INDEX;
		}
	}
	return check;
}


//-------------------------------------------------------------------------------------------------
bool NetSession::IsConnected( ) const
{
	return m_channel.IsConnected( );
}


//-------------------------------------------------------------------------------------------------
bool NetSession::IsValidPacket( NetPacket const & packet, size_t packetSize ) const
{
	PacketHeader header;
	packet.ReadHeader( &header );

	size_t totalSize = packet.GetCurrentOffset( );
	//Stride over each message and sum up the size
	for( uint8_t messageNum = 0; messageNum < header.messageCount; ++messageNum )
	{
		uint16_t messageSize;
		packet.Read<uint16_t>( &messageSize );
		totalSize += sizeof(uint16_t) + messageSize; //size variable + reported size
		if( totalSize > packetSize )
		{
			return false;
		}
		packet.Advance( messageSize );
	}

	packet.Rewind( );

	//Size must match total size
	return totalSize == packetSize;
}


//-------------------------------------------------------------------------------------------------
bool NetSession::IsValidMessage( NetSender const & senderInfo, NetMessage const & message ) const
{
	bool valid = true;

	//Definition doesn't exist
	if( !message.m_definition )
	{
		valid = false;
	}
	else if( message.m_definition->IsReliable( ) )
	{
		if( message.m_reliableID == NetMessage::INVALID_RELIABLE_ID )
		{
			valid = false;
		}
	}

	if( !message.m_definition->IsConnectionless( ) )
	{
		//Needs connection and connection == nullptr
		if( !senderInfo.connection )
		{
			valid = false;
		}
		//Needs connection but connection index doesn't exist
		else if( !IsValidConnectionIndex( message.m_senderIndex ) )
		{
			valid = false;
		}
		//from index and written index doesn't match
		else if( senderInfo.connection->GetIndex( ) != message.m_senderIndex )
		{
			valid = false;
		}
	}
	
	return valid;
}


//-------------------------------------------------------------------------------------------------
bool NetSession::IsValidConnectionIndex( byte_t index ) const
{
	if( index < MAX_CONNECTIONS )
	{
		return m_connections[index] != nullptr;
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
bool NetSession::IsDuplicateGUID( std::string const & check ) const
{
	for( byte_t connIndex = 0; connIndex < MAX_CONNECTIONS; ++connIndex )
	{
		//Skip connections that don't exist
		if( !m_connections[connIndex] )
		{
			continue;
		}

		if( strcmp( m_connections[connIndex]->GetGUID( ), check.c_str( ) ) == 0 )
		{
			return true;
		}
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
bool NetSession::IsHost( ) const
{
	return m_self == m_host && m_host != nullptr;
}


//-------------------------------------------------------------------------------------------------
void NetSession::SetDropRate( float dropRate )
{
	m_channel.m_dropRate = dropRate;
}


//-------------------------------------------------------------------------------------------------
void NetSession::SetLatency( Range<double> latency )
{
	m_channel.m_latency = latency;
}


//-------------------------------------------------------------------------------------------------
bool NetSession::ToggleTimeouts( )
{
	m_connectionTimeouts = !m_connectionTimeouts;
	return m_connectionTimeouts;
}