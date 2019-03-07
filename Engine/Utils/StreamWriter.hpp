#pragma once

#include "Engine/Core/EngineCommon.hpp"

//-------------------------------------------------------------------------------------------------
class StreamWriter
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	byte_t * m_buffer;
	size_t m_head;
	size_t m_capacity;
	bool m_isDynamic;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	StreamWriter( bool isDynamic = true );
	StreamWriter( int capacity, bool isDynamic = true );
	StreamWriter( size_t capacity, bool isDynamic = true );
	StreamWriter( byte_t * buffer, size_t capacity, bool isDynamic = false );
	~StreamWriter( );

	void Write( void const * data, size_t dataByteSize );
	void Reset( );
	byte_t const * GetBuffer( ) const;
	size_t GetSize( ) const;

private:
	void Reallocate( size_t capacity );

//-------------------------------------------------------------------------------------------------
// Template Functions
//-------------------------------------------------------------------------------------------------
public:
	template<typename DataType>
	void Write( DataType const & data )
	{
		Write( &data, sizeof( DataType ) );
	}
};