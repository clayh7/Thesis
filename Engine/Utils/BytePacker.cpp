#include "Engine/Utils/BytePacker.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
BytePacker::BytePacker( byte_t * buffer, size_t bufferSize )
	: BytePacker( buffer, bufferSize, bufferSize )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
BytePacker::BytePacker( byte_t * buffer, size_t bufferMax, size_t bufferSize )
	: m_buffer( buffer )
	, m_bufferMax( bufferMax )
	, m_bufferSize( bufferSize )
	, m_offset( 0 )
	, m_endianness( eEndianMode_LITTLE_ENDIAN )
{
	if( bufferMax < bufferSize )
	{
		ERROR_AND_DIE( "Buffer size too large" );
	}
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::Write( byte_t const * data, size_t dataSize )
{
	if( GetLocalEndianess( ) == m_endianness )
	{
		return WriteForward( data, dataSize );
	}
	else
	{
		return WriteBackwards( data, dataSize );
	}
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::WriteForward( byte_t const * data, size_t dataSize )
{
	//Make sure we don't go out of bounds
	if( m_offset + dataSize - 1 >= m_bufferMax )
	{
		return false;
	}

	//Write the data
	memcpy( &m_buffer[m_offset], data, dataSize );
	
	//Advance the offset
	m_offset += dataSize;
	m_bufferSize += dataSize;
	return true;
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::WriteBackwards( byte_t const * data, size_t dataSize )
{
	//Make sure we don't go out of bounds
	if( m_offset + dataSize - 1 >= m_bufferMax )
	{
		return false;
	}

	//Write the data backwards
	data += dataSize - 1;
	for( size_t index = 0; index < dataSize; ++index )
	{
		m_buffer[m_offset + index] = *data;
		--data;
	}

	//Advance the offset
	m_offset += dataSize;
	m_bufferSize += dataSize;
	return true;
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::WriteString( char const * data )
{
	if( data == nullptr )
	{
		return Write<byte_t>( NULL_CHAR );
	}
	else
	{
		size_t len = strlen( data ) + 1; //include null terminator
		return WriteForward( (byte_t*) data, len );
	}
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::Read( byte_t * out_data, size_t readAmount ) const
{
	if( GetLocalEndianess( ) == m_endianness )
	{
		return ReadForward( out_data, readAmount );
	}
	else
	{
		return ReadBackwards( out_data, readAmount );
	}
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::ReadForward( byte_t * out_data, size_t readAmount ) const
{
	//Make sure we don't go out of bounds
	if( m_offset + readAmount - 1 >= m_bufferSize )
	{
		return false;
	}

	//Read data from buffer
	memcpy( out_data, &m_buffer[m_offset], readAmount );

	//Advance offset
	m_offset += readAmount;
	return true;
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::ReadBackwards( byte_t * out_data, size_t readAmount ) const
{
	//Make sure we don't go out of bounds
	if( m_offset + readAmount - 1 >= m_bufferSize )
	{
		return false;
	}

	//Read data from buffer
	out_data += readAmount - 1;
	for( size_t index = 0; index < readAmount; ++index )
	{
		*out_data = m_buffer[m_offset + index];
		--out_data;
	}

	//Advance offset
	m_offset += readAmount;
	return true;
}


//-------------------------------------------------------------------------------------------------
bool BytePacker::ReadString( char ** out_data ) const
{
	if( GetReadableBytesLeft( ) < 1 )
	{
		*out_data = nullptr;
		return false;
	}

	byte_t check;
	Read( &check );
	if( check == NULL_CHAR )
	{
		*out_data = nullptr;
	}
	else
	{
		*out_data = (char*)GetHead() - 1;
		size_t max_size = GetReadableBytesLeft( ) + 1;
		size_t length = 0;
		char * c = *out_data;
		while( ( *c != NULL ) && ( length < max_size ) )
		{
			++length;
			++c;
		}
		if( length < max_size )
		{
			Advance( length );
		}
		else
		{
			*out_data = nullptr;
			return false;
		}
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
void BytePacker::Advance( size_t amount ) const
{
	m_offset += amount;
}


//-------------------------------------------------------------------------------------------------
size_t BytePacker::GetWritableBytesLeft( ) const
{
	return m_bufferMax - m_bufferSize;
}


//-------------------------------------------------------------------------------------------------
size_t BytePacker::GetReadableBytesLeft( ) const
{
	return m_bufferSize - m_offset;
}


//-------------------------------------------------------------------------------------------------
size_t BytePacker::GetCurrentOffset( ) const
{
	return m_offset;
}


//-------------------------------------------------------------------------------------------------
byte_t * BytePacker::GetBuffer( ) const
{
	return m_buffer;
}


//-------------------------------------------------------------------------------------------------
byte_t * BytePacker::GetHead( ) const
{
	return &m_buffer[m_offset];
}


//-------------------------------------------------------------------------------------------------
void BytePacker::SetBufferSize( size_t size )
{
	m_bufferSize = size;
}


//-------------------------------------------------------------------------------------------------
void BytePacker::Rewind( ) const
{
	m_offset = 0;
}
