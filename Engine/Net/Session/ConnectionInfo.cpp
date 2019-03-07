#include "Engine/Net/Session/ConnectionInfo.hpp"

#include "Engine/Net/Session/NetSession.hpp"


//-------------------------------------------------------------------------------------------------
ConnectionInfo::ConnectionInfo( )
	: m_index( NetSession::INVALID_INDEX )
	, m_address( )
	, m_guid( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
ConnectionInfo::ConnectionInfo( byte_t idx, sockaddr_in const & addr, char const * id, char const * name )
	: m_index( idx )
	, m_address( addr )
{
	// if it was MAX_GUID_SIZE + 1 it would go over bounds, but we need that null
	size_t stringLength = strnlen_s( id, MAX_GUID_SIZE - 1 ) + 1;
	memcpy( m_guid, id, stringLength );

	stringLength = strnlen_s( name, MAX_USERNAME_SIZE - 1 ) + 1;
	memcpy( m_username, name, stringLength );
}