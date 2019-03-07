#pragma once


#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class ByteQueue
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const int SIZE = 1024;
	static byte_t * s_queue;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void SystemInit( byte_t buffer[SIZE] );
	static ByteQueue * CreateQueue( );
	static void DestroyQueue( ByteQueue * queue );
	static void Push( ByteQueue * queue, byte_t byte );
	static byte_t Pop( ByteQueue * queue );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	int size; //Assuming I can make an arbitrary constraint
	//Otherwise with int (or nibble w/e)
};