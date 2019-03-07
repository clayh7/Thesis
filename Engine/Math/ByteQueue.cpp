#include "Engine/Math/ByteQueue.hpp"


//-------------------------------------------------------------------------------------------------
void ByteQueue::SystemInit( byte_t buffer[SIZE] )
{
	s_queue = buffer;
	ByteQueue * head = (ByteQueue *) s_queue;
	head->size = 0;
}


//-------------------------------------------------------------------------------------------------
ByteQueue * ByteQueue::CreateQueue( )
{
	ByteQueue * head = (ByteQueue *) s_queue;
	int qCount = head->size;

	if( qCount == sizeof( ByteQueue ) - 1 )
		return nullptr; // ERROR!

	head[qCount].size = 0;
	head->size += 1;
	return &head[qCount];
}


//-------------------------------------------------------------------------------------------------
void ByteQueue::DestroyQueue( ByteQueue * queue )
{
	ByteQueue * head = (ByteQueue *) s_queue;
	int qCount = head->size;
	int destroySize = queue->size;

	if( qCount == 0 )
		return; // ERROR!

	ByteQueue * currentQueue = &head[1];
	int queueIndex = 0;
	int queueDepth = 0;
	while( currentQueue != queue )
	{
		queueDepth += currentQueue->size;
		++currentQueue;
		++queueIndex;
		if( queueIndex > qCount )
			return; // ERROR!
	}

	while( queueIndex < qCount )
	{
		head[queueIndex] = head[queueIndex++];
	}
	head->size -= 1;

	byte_t * start = &s_queue[SIZE - queueDepth];
	byte_t * end = start + destroySize;
	while( end != (byte_t*) &head[head->size] )
	{
		*start = *end;
		--start;
		--end;
	}
}


//-------------------------------------------------------------------------------------------------
void ByteQueue::Push( ByteQueue * queue, byte_t byte )
{
	//Check current size
	ByteQueue * head = (ByteQueue *) s_queue;
	int qCount = head->size;
	int total = 0;
	for( int qIndex = 1; qIndex < qCount; ++qIndex )
	{
		total += head[qIndex].size;
	}
	total += ( qCount + 1 ) * sizeof( ByteQueue );
	if( total >= SIZE )
		return; // ERROR!

	queue->size += 1;

	//Find push location
	byte_t * back = &s_queue[SIZE - 1];
	ByteQueue * currentQueue = queue - 1;
	while( (byte_t *) currentQueue > s_queue )
	{
		back -= currentQueue->size;
		--currentQueue;
	}

	byte_t * end = (byte_t*) &head[qCount + 1];

	//Shift all the way to back
	while( back != (byte_t*) end )
	{
		*( end ) = *( end + 1 );
		++end;
	}

	*end = byte;
}


//-------------------------------------------------------------------------------------------------
byte_t ByteQueue::Pop( ByteQueue * queue )
{
	//Check current size
	if( queue->size == 0 )
		return 0; // ERROR!

	ByteQueue * currentQueue = queue;
	byte_t * back = &s_queue[SIZE - 1];

	//Find pop location
	while( (byte_t *) currentQueue > s_queue )
	{
		back -= currentQueue->size;
		--currentQueue;
	}

	byte_t result = *back;
	byte_t qCount = s_queue[0];
	ByteQueue * end = ( ( (ByteQueue*) ( s_queue + 1 ) ) + qCount );

	//Shift it all down
	while( back != (byte_t*) end )
	{
		*back = *( back - 1 );
		--back;
	}

	return result;
}