#pragma once

#include <map>
#include "Engine/Net/UDPIP/UDPSock.hpp"
#include "Engine/Math/Range.hpp"


//-------------------------------------------------------------------------------------------------
class NetSession;
class NetPacket;
class PacketHeader;


//-------------------------------------------------------------------------------------------------
class PacketChannel : public UDPSock
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	float m_dropRate;
	Range<double> m_latency;

private:
	std::map<double, NetPacket*> m_orderedPackets;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	PacketChannel( );
	~PacketChannel( );

	void SendPackets( sockaddr_in addr, byte_t const * data, size_t dataSize ) const;
	void RecvPackets( NetSession * currentSession );
};