#include "Engine/Utils/StreamReader.hpp"

#include "Engine/Utils/StreamWriter.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
StreamReader::StreamReader( )
	: m_buffer( nullptr )
	, m_head( 0U )
	, m_capacity( 0U )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
StreamReader::StreamReader( byte_t const * buffer, size_t capacity )
	: m_buffer( buffer )
	, m_head( 0U )
	, m_capacity( capacity )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
StreamReader::StreamReader( StreamWriter const & writer )
	: m_buffer( writer.GetBuffer( ) )
	, m_head( 0U )
	, m_capacity( writer.GetSize( ) )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
StreamReader::StreamReader( StreamWriter const * writer )
	: m_buffer( nullptr )
	, m_head( 0U )
	, m_capacity( 0U )
{
	SetBuffer( writer );
}


//-------------------------------------------------------------------------------------------------
void StreamReader::Read( void * out_data, size_t dataByteCount )
{
	ASSERT_OR_DIE( out_data != nullptr, "StreamReader: Cannot output to nullptr." )
	ASSERT_OR_DIE( m_head + dataByteCount <= m_capacity, "StreamReader: Trying to read outside the buffer." );
	memcpy( out_data, m_buffer + m_head, dataByteCount );
	m_head += dataByteCount;
}


//-------------------------------------------------------------------------------------------------
void StreamReader::SetBuffer( byte_t const * buffer, size_t capacity )
{
	m_buffer = buffer;
	m_capacity = capacity;
}


//-------------------------------------------------------------------------------------------------
void StreamReader::SetBuffer( StreamWriter const & writer )
{
	SetBuffer( writer.GetBuffer( ), writer.GetSize( ) );
}


//-------------------------------------------------------------------------------------------------
void StreamReader::SetBuffer( StreamWriter const * writer )
{
	if( writer != nullptr )
	{
		SetBuffer( *writer );
	}
}


//-------------------------------------------------------------------------------------------------
void StreamReader::Reset( )
{
	m_head = 0U;
}
