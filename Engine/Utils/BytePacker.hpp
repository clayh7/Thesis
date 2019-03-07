#pragma once

#include "Engine/Utils/EndianUtils.hpp"


//-------------------------------------------------------------------------------------------------
class BytePacker
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static byte_t const NULL_CHAR = 0xff;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
protected:
	byte_t * m_buffer;
	size_t m_bufferMax;
	size_t m_bufferSize;
	mutable size_t m_offset; //Means it can be changed even if you call something const (see Read())
	eEndianMode m_endianness;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	BytePacker( byte_t * buffer, size_t bufferSize );
	BytePacker( byte_t * buffer, size_t bufferMax, size_t bufferSize );

	template<typename DataType>
	bool Write( DataType const & data )
	{
		return Write( (byte_t*)&data, sizeof( DataType ) );
	}
	bool Write( byte_t const * data, size_t dataSize );
	bool WriteForward( byte_t const * data, size_t dataSize );
	bool WriteBackwards( byte_t const * data, size_t dataSize );
	bool WriteString( char const * data );

	template<typename DataType>
	size_t Reserve( DataType const & data )
	{
		size_t currentPosition = m_offset;
		Write<DataType>( data );
		return currentPosition;
	}
	template<typename DataType>
	//#TODO: rewrite this function
	void WriteAt( size_t position, DataType const & data )
	{
		size_t offset = m_offset;
		size_t size = m_bufferSize;
		Rewind( );
		Advance( position );
		Write<DataType>( data );
		m_offset = offset;
		m_bufferSize = size;
	}

	template<typename DataType>
	bool Read( DataType * out_data ) const
	{
		return Read( (byte_t*) out_data, sizeof( DataType ) );
	}
	bool Read( byte_t * out_data, size_t readAmount ) const;
	bool ReadForward( byte_t * out_data, size_t readAmount ) const;
	bool ReadBackwards( byte_t * out_data, size_t readAmount ) const;
	bool ReadString( char ** out_data ) const;
	void Advance( size_t amount ) const;

	size_t GetWritableBytesLeft( ) const;
	size_t GetReadableBytesLeft( ) const;
	size_t GetCurrentOffset( ) const;
	byte_t * GetBuffer( ) const;
	byte_t * GetHead( ) const;

	void SetBufferSize( size_t size );
	void Rewind( ) const;
};