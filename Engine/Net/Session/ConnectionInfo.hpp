#pragma once

#include "Engine/Core/EngineCommon.hpp"
//#include "Engine/Net/NetworkSystem.hpp"
#include "Engine/Utils/NetworkUtils.hpp"


//-------------------------------------------------------------------------------------------------
class ConnectionInfo
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static size_t const MAX_GUID_SIZE = 32;
	static size_t const MAX_USERNAME_SIZE = 32;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	byte_t m_index;
	sockaddr_in m_address;
	char m_guid[MAX_GUID_SIZE];
	char m_username[MAX_USERNAME_SIZE];
	size_t m_password;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	ConnectionInfo( );
	ConnectionInfo( byte_t index, sockaddr_in const & address, char const * id, char const * name );
};