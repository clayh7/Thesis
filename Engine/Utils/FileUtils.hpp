#pragma once

#include <string>
#include <vector>
#include "Engine/Utils/EndianUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
// Binary Reader/Writer Interfaces
//-------------------------------------------------------------------------------------------------
class IBinaryReader
{
private:
	eEndianMode endianMode;

public:
	void SetEndianess( eEndianMode mode ) { endianMode = mode; }

	virtual size_t ReadBytes( void *out_buffer, size_t const numBytes ) = 0;
	virtual size_t ReadString( std::string *out_buffer ) = 0;
	virtual size_t ReadFloats( std::vector<float> *out_buffer ) = 0;
	virtual size_t ReadInts( std::vector<int> *out_buffer ) = 0;

	template <typename DataType>
	bool Read( DataType *data )
	{
		//#TODO: add Swapping for Endianness
		return ReadBytes( data, sizeof( DataType ) ) == sizeof( DataType );
	}
};


//-------------------------------------------------------------------------------------------------
class IBinaryWriter
{
private:
	eEndianMode endianMode;

public:
	void SetEndianess( eEndianMode mode ) { endianMode = mode; }

	virtual size_t WriteBytes( void const *src, size_t const numBytes ) = 0;
	virtual bool WriteString( std::string const &string ) = 0;
	virtual bool WriteFloats( std::vector<float> const &floats ) = 0;
	virtual bool WriteInts( std::vector<float> const &ints ) = 0;

	template<typename DataType>
	bool Write( DataType const &data )
	{
		//#TODO: add Swapping for Endianness
// 		DataType copy = v;
// 		if ( GetLocalEndianess( ) == endianMode )
// 		{
// 			ByteSwap( &copy, sizeof( DataType ) );
// 		}
		return WriteBytes( &data, sizeof( DataType ) ) == sizeof( DataType );
	}
};


//-------------------------------------------------------------------------------------------------
// File Reader/Writer Classes
//-------------------------------------------------------------------------------------------------
class FileBinaryReader
	: public IBinaryReader
{
public:
	FILE *fileHandle;

public:
	FileBinaryReader( );

	bool Open( std::string const &filename );
	void Close( );

public:
	virtual size_t ReadBytes( void *out_buffer, size_t const numBytes ) override;
	virtual size_t ReadString( std::string *out_buffer ) override;
	virtual size_t ReadFloats( std::vector<float> *out_buffer ) override;
	virtual size_t ReadInts( std::vector<int> *out_buffer ) override;
};


//-------------------------------------------------------------------------------------------------
class FileBinaryWriter
	: public IBinaryWriter
{
public:
	FILE *fileHandle;

public:
	FileBinaryWriter( );

	bool Open( std::string const &filename, bool append = false );
	void Close( );

public:
	virtual size_t WriteBytes( void const *src, size_t const numBytes ) override;
	virtual bool WriteString( std::string const &string ) override;
	virtual bool WriteFloats( std::vector<float> const &floats ) override;
	virtual bool WriteInts( std::vector<float> const &ints ) override;
};


//-------------------------------------------------------------------------------------------------
//put this in a bit utils class
void ByteSwap( void *data, size_t const dataSize );


//-------------------------------------------------------------------------------------------------
std::vector<std::string> EnumerateFilesInFolder( std::string const &relativeDirectoryPath, std::string const filePattern );


//-----------------------------------------------------------------------------------------------
// Old Read/Write Functions
//-------------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer( std::string const & filePath, std::string& out_buffer );
bool SaveBufferToBinaryFile( std::string const &filePath, std::string const &buffer );
bool LoadBinaryFileToBuffer( std::string const & filePath, std::vector< unsigned char >& out_buffer );
bool SaveBufferToBinaryFile( std::string const & filePath, const std::vector< unsigned char >& buffer );
bool LoadBinaryFileToBuffer( std::string const & filePath, std::vector< float >& out_buffer );
bool SaveBufferToBinaryFile( std::string const & filePath, const std::vector< float >& buffer );