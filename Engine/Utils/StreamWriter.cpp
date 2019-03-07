#include "Engine/Utils/StreamWriter.hpp"

#include "Engine/Utils/MathUtils.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
StreamWriter::StreamWriter( bool isDynamic /*= true */ )
	: StreamWriter( 32U, isDynamic )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
StreamWriter::StreamWriter( int capacity, bool isDynamic /*= true */ )
	: StreamWriter( static_cast<size_t>( capacity ), isDynamic )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
StreamWriter::StreamWriter( size_t capacity, bool isDynamic /*= true */ )
	: m_buffer( nullptr )
	, m_head( 0U )
	, m_capacity( capacity )
	, m_isDynamic( isDynamic )
{
	Reallocate( capacity );
}


//-------------------------------------------------------------------------------------------------
StreamWriter::StreamWriter( byte_t * buffer, size_t capacity, bool isDynamic /*= false */ )
	: m_buffer( buffer )
	, m_head( 0U )
	, m_capacity( capacity )
	, m_isDynamic( isDynamic )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
StreamWriter::~StreamWriter( )
{
	if( m_buffer != nullptr )
	{
		free( m_buffer );
	}
}


//-------------------------------------------------------------------------------------------------
void StreamWriter::Write( void const * data, size_t dataByteSize )
{
	size_t nextHead = m_head + dataByteSize;
	if( nextHead > m_capacity )
	{
		size_t newCapacity = Max( nextHead, m_capacity * 2U );
		Reallocate( newCapacity );
	}

	memcpy( m_buffer + m_head, data, dataByteSize );
	m_head = nextHead;
}


//-------------------------------------------------------------------------------------------------
void StreamWriter::Reset( )
{
	m_head = 0U;
}


//-------------------------------------------------------------------------------------------------
byte_t const * StreamWriter::GetBuffer( ) const
{
	return m_buffer;
}


//-------------------------------------------------------------------------------------------------
size_t StreamWriter::GetSize( ) const
{
	return m_head;
}


//-------------------------------------------------------------------------------------------------
void StreamWriter::Reallocate( size_t capacity )
{
	ASSERT_OR_DIE( m_isDynamic, "StreamWriter: Attempting to reallocate non-dynamic." );
	byte_t * temp = static_cast<byte_t*>( malloc( capacity ) );
	if( m_buffer != nullptr )
	{
		memcpy( temp, m_buffer, m_capacity );
		free( m_buffer );
	}
	m_buffer = temp;
	m_capacity = capacity;
}
