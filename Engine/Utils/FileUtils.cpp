#include "Engine/Utils/FileUtils.hpp"

#include <io.h>
#include <stdio.h>
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
FileBinaryReader::FileBinaryReader( )
	: fileHandle( nullptr )
{
	SetEndianess( GetLocalEndianess( ) );
}


//-------------------------------------------------------------------------------------------------
bool FileBinaryReader::Open( std::string const &filename )
{
	std::string mode = "rb"; //I don't know any other mode

	errno_t error = fopen_s( &fileHandle, filename.c_str( ), mode.c_str( ) );
	if ( error != 0 )
	{
		return false;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void FileBinaryReader::Close( )
{
	if ( fileHandle != nullptr )
	{
		fclose( fileHandle );
		fileHandle = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
size_t FileBinaryReader::ReadBytes( void *out_buffer, size_t const numBytes )
{
	return fread( out_buffer, sizeof( byte_t ), numBytes, fileHandle );
}


//-------------------------------------------------------------------------------------------------
size_t FileBinaryReader::ReadString( std::string *out_buffer )
{
	uint32_t numBytes;
	Read<uint32_t>( &numBytes );
	out_buffer->resize( numBytes );
	return ReadBytes( &( *out_buffer )[0], numBytes );
}


//-------------------------------------------------------------------------------------------------
size_t FileBinaryReader::ReadFloats( std::vector<float> *out_buffer )
{
	uint32_t numBytes;
	Read<uint32_t>( &numBytes );
	out_buffer->resize( numBytes );
	return ReadBytes( out_buffer, numBytes );
}


//-------------------------------------------------------------------------------------------------
size_t FileBinaryReader::ReadInts( std::vector<int> *out_buffer )
{
	uint32_t numBytes;
	Read<uint32_t>( &numBytes );
	out_buffer->resize( numBytes );
	return ReadBytes( out_buffer, numBytes );
}


//-------------------------------------------------------------------------------------------------
FileBinaryWriter::FileBinaryWriter( )
	: fileHandle( nullptr )
{
	SetEndianess( GetLocalEndianess( ) );
}


//-------------------------------------------------------------------------------------------------
bool FileBinaryWriter::Open( std::string const &filename, bool append /*= false */ )
{
	std::string mode;
	if ( append )
	{
		mode = "ab"; //append
	}
	else
	{
		mode = "wb"; //write
	}

	errno_t error = fopen_s( &fileHandle, filename.c_str( ), mode.c_str( ) );
	if ( error != 0 )
	{
		return false;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------
void FileBinaryWriter::Close( )
{
	if ( fileHandle != nullptr )
	{
		fclose( fileHandle );
		fileHandle = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
size_t FileBinaryWriter::WriteBytes( void const *src, size_t const numBytes )
{
	return fwrite( src, sizeof( byte_t ), numBytes, fileHandle );
}


//-------------------------------------------------------------------------------------------------
bool FileBinaryWriter::WriteString( std::string const &string )
{
	size_t length = string.size( );
	size_t bufferLength = length + 1;
	//Write how much data I'm going to write, then write the data
	return Write<uint32_t>( bufferLength ) && ( WriteBytes( &string[0], bufferLength ) == bufferLength );
}


//-------------------------------------------------------------------------------------------------
bool FileBinaryWriter::WriteFloats( std::vector<float> const &floats )
{
	size_t length = floats.size( );
	size_t bufferLength = length;
	//Write how much data I'm going to write, then write the data
	return Write<uint32_t>( bufferLength ) && ( WriteBytes( &floats[0], length ) == length );
}


//-------------------------------------------------------------------------------------------------
bool FileBinaryWriter::WriteInts( std::vector<float> const &ints )
{
	size_t length = ints.size( );
	size_t bufferLength = length;
	//Write how much data I'm going to write, then write the data
	return Write<uint32_t>( bufferLength ) && ( WriteBytes( &ints[0], length ) == length );
}


//-------------------------------------------------------------------------------------------------
void ByteSwap( void *data, size_t const dataSize )
{
	//#TODO: Finish Function
	UNREFERENCED( data );
	UNREFERENCED( dataSize );
}


//-------------------------------------------------------------------------------------------------
std::vector<std::string> EnumerateFilesInFolder( std::string const &relativeDirectoryPath, std::string const filePattern )
{
	std::string	searchPathPattern = relativeDirectoryPath + filePattern;
	std::vector< std::string > foundFiles;

	int error = 0;
	struct _finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( searchPathPattern.c_str( ), &fileInfo );
	while ( searchHandle != -1 && !error )
	{
		std::string relativePathToFile = Stringf( "%s%s", relativeDirectoryPath.c_str( ), fileInfo.name );
		bool isDirectory = fileInfo.attrib & _A_SUBDIR ? true : false;
		bool isHidden = fileInfo.attrib & _A_HIDDEN ? true : false;

		if ( !isDirectory && !isHidden )
		{
			foundFiles.push_back( relativePathToFile );
		}

		error = _findnext( searchHandle, &fileInfo );
	}
	_findclose( searchHandle );

	return foundFiles;
}


//-------------------------------------------------------------------------------------------------
// Old Read Write Functions
//-------------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer( std::string const & filePath, std::string& out_buffer )
{
	//Get file pointer
	FILE* ptrToFile = nullptr;
	errno_t errorSuccess = fopen_s( &ptrToFile, filePath.c_str( ), "rb" );

	//Make sure it opened something
	if ( errorSuccess != 0 )
		return false;

	//Reading
	fseek( ptrToFile, 0, SEEK_END );
	size_t sizeOfFile = ftell( ptrToFile );
	rewind( ptrToFile );
	out_buffer.resize( sizeOfFile );
	fread( &out_buffer[0], sizeof( char ), sizeOfFile, ptrToFile );

	//Always close
	fclose( ptrToFile );

	return true;
}


//-------------------------------------------------------------------------------------------------
bool SaveBufferToBinaryFile( std::string const &filePath, std::string const &buffer )
{
	//Get file pointer
	FILE* ptrToFile = nullptr;
	errno_t errorSuccess = fopen_s( &ptrToFile, filePath.c_str( ), "wb" );

	//Make sure it opened something
	if ( errorSuccess != 0 )
		return false;

	//Writing
	fwrite( &buffer[0], sizeof( char ), buffer.size( ), ptrToFile );

	//Always close
	fclose( ptrToFile );

	return true;
}


//-------------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer( std::string const & filePath, std::vector< unsigned char >& out_buffer )
{
	//Get file pointer
	FILE* ptrToFile = nullptr;
	errno_t errorSuccess = fopen_s( &ptrToFile, filePath.c_str( ), "rb" );

	//Make sure it opened something
	if ( errorSuccess != 0 )
		return false;

	//Reading
	fseek( ptrToFile, 0, SEEK_END );
	size_t sizeOfFile = ftell( ptrToFile );
	rewind( ptrToFile );
	out_buffer.resize( sizeOfFile );
	fread( &out_buffer[0], sizeof(unsigned char), sizeOfFile, ptrToFile );

	//Always close
	fclose( ptrToFile );

	return true;
}


//-------------------------------------------------------------------------------------------------
bool SaveBufferToBinaryFile( std::string const & filePath, const std::vector< unsigned char >& buffer )
{
	//Get file pointer
	FILE* ptrToFile = nullptr;
	errno_t errorSuccess = fopen_s( &ptrToFile, filePath.c_str( ), "wb" );

	//Make sure it opened something
	if ( errorSuccess != 0 )
		return false;

	//Writing
	fwrite( &buffer[0], sizeof(unsigned char), buffer.size( ), ptrToFile );

	//Always close
	fclose( ptrToFile );

	return true;
}


//-------------------------------------------------------------------------------------------------
bool LoadBinaryFileToBuffer( std::string const & filePath, std::vector< float >& out_buffer )
{
	//Get file pointer
	FILE* ptrToFile = nullptr;
	errno_t errorSuccess = fopen_s( &ptrToFile, filePath.c_str( ), "rb" );

	//Make sure it opened something
	if ( errorSuccess != 0 )
		return false;

	//Reading
	fseek( ptrToFile, 0, SEEK_END );
	size_t sizeOfFile = ftell( ptrToFile );
	rewind( ptrToFile );
	out_buffer.resize( sizeOfFile );
	fread( &out_buffer[0], sizeof(float), sizeOfFile, ptrToFile );

	//Always close
	fclose( ptrToFile );

	return true;
}


//-------------------------------------------------------------------------------------------------
bool SaveBufferToBinaryFile( std::string const & filePath, const std::vector< float >& buffer )
{
	//Get file pointer
	FILE* ptrToFile = nullptr;
	errno_t errorSuccess = fopen_s( &ptrToFile, filePath.c_str( ), "wb" );

	//Make sure it opened something
	if ( errorSuccess != 0 )
		return false;

	//Writing
	fwrite( &buffer[0], sizeof(float), buffer.size( ), ptrToFile );

	//Always close
	fclose( ptrToFile );

	return true;
}