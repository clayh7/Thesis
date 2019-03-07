#include "Engine/Net/Session/PacketChannel.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Net/Session/NetPacket.hpp"
#include "Engine/Net/Session/NetSession.hpp"
#include "Engine/Net/Session/NetConnection.hpp"


//-------------------------------------------------------------------------------------------------
PacketChannel::PacketChannel( )
	: UDPSock( )
	, m_dropRate( 0.f )
	, m_latency( Range<double>::ZERO )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
PacketChannel::~PacketChannel( )
{
	for( auto packetIter : m_orderedPackets )
	{
		delete packetIter.second;
		packetIter.second = nullptr;
	}
	m_orderedPackets.clear( );
}


//-------------------------------------------------------------------------------------------------
void PacketChannel::SendPackets( sockaddr_in addr, byte_t const * data, size_t dataSize ) const
{
	Send( addr, data, dataSize );
}

//-------------------------------------------------------------------------------------------------
void PacketChannel::RecvPackets( NetSession * currentSession )
{
	double currentTime = Time::GetCurrentTimeSeconds( );

	NetPacket packet;
	sockaddr_in address;
	size_t read = Recv( &address, packet.GetBuffer( ), NetPacket::MAX_SIZE );
	packet.SetBufferSize( read );

	packet.m_senderInfo.session = currentSession;
	
	//Recieve Packets
	while( read > 0 )
	{
		//Packet is Invalid
		if( !currentSession->IsValidPacket( packet, read ) )
		{
			read = Recv( &address, packet.GetBuffer( ), NetPacket::MAX_SIZE );
			++currentSession->m_invalidPacketCount;
			continue;
		}

		//Skip packet if within drop rate
		if( RandomFloatZeroToOne( ) >= m_dropRate )
		{
			packet.m_senderInfo.fromAddress = address;
			if( m_latency == Range<double>::ZERO )
			{
				currentSession->ProcessPacket( packet );
			}
			else
			{
				double readTime = currentTime + m_latency.GetRandom( );
				m_orderedPackets.insert( std::pair<double, NetPacket*>( readTime, packet.Copy() ) );
			}
		}

		//Continue to the next packet
		read = Recv( &address, packet.GetBuffer( ), NetPacket::MAX_SIZE );
		packet.Rewind( );
		packet.SetBufferSize( read );
	}
	
	//Process Packets
	if( m_orderedPackets.size( ) > 0 )
	{
		for( auto packetIter = m_orderedPackets.begin( ); packetIter != m_orderedPackets.end( ); /*Nothing*/ )
		{
			if( packetIter->first <= currentTime )
			{
				currentSession->ProcessPacket( *(packetIter->second) );
				delete packetIter->second;
				packetIter->second = nullptr;
				packetIter = m_orderedPackets.erase( packetIter );
			}
			else
			{
				break;
			}
		}
	}
}