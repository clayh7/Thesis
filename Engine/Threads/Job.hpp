#pragma once

#include <atomic>
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
enum eJobCategory
{
	eJobCategory_GENERIC, //Can run on the main thread
	eJobCategory_GENERIC_SLOW, //Should NOT run on the main thread
	eJobCategory_COUNT,
};


//-------------------------------------------------------------------------------------------------
class Job;


//-------------------------------------------------------------------------------------------------
typedef void ( JobCallback )( Job * );


//-------------------------------------------------------------------------------------------------
class Job
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const int BUFFER_SIZE = 512;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	byte_t m_byteBuffer[BUFFER_SIZE];
	size_t m_readHead; //This is the index for how far you are in byte buffer
	size_t m_writeHead; //When done with reading and writing set back to 0

public:
	int * bob;
	eJobCategory m_category;
	JobCallback * m_jobFunc;
	std::atomic<int> m_refCount;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Job( );

	void Work( );

//-------------------------------------------------------------------------------------------------
// Function Templates
//-------------------------------------------------------------------------------------------------
	template<typename DataType>
	void Write( DataType const & data )
	{
		byte_t * destination = m_byteBuffer + m_writeHead;
		size_t size = sizeof( DataType );
		memcpy( destination, &data, size );
		m_writeHead += size;
	}

	template<typename DataType>
	DataType Read( )
	{
		DataType data;
		byte_t * source = m_byteBuffer + m_readHead;
		size_t size = sizeof( DataType );
		memcpy( &data, source, size );
		m_readHead += size;
		return data;
	}
};