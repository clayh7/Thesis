#pragma once

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class StreamWriter;


//-------------------------------------------------------------------------------------------------
class StreamReader
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	byte_t const * m_buffer;
	size_t m_head;
	size_t m_capacity;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	StreamReader( );
	StreamReader( byte_t const * buffer, size_t capacity );
	StreamReader( StreamWriter const & writer );
	StreamReader( StreamWriter const * writer );

	void Read( void * out_data, size_t dataByteCount );
	void SetBuffer( byte_t const * buffer, size_t capacity );
	void SetBuffer( StreamWriter const & writer );
	void SetBuffer( StreamWriter const * writer );
	void Reset( );

//-------------------------------------------------------------------------------------------------
// Template Functions
//-------------------------------------------------------------------------------------------------
public:
	template<typename DataType>
	void Read( DataType * out_data )
	{
		Read( out_data, sizeof( DataType ) );
	}
};