#pragma once
#if !defined( __DEBUG_CALLSTACK__ )
#define __DEBUG_CALLSTACK__


//-------------------------------------------------------------------------------------------------
class Callstack
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	void ** frameDataPtr;
	unsigned int frame_count;
};


//-------------------------------------------------------------------------------------------------
class CallstackLine
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static int const MAX_SYMBOL_NAME_LENGTH = 128;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	char filename[MAX_SYMBOL_NAME_LENGTH];
	char function_name[MAX_SYMBOL_NAME_LENGTH];
	unsigned int line;
	unsigned int offset;
};


//-------------------------------------------------------------------------------------------------
class CallstackSystem
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static int const MAX_FILENAME_LENGTH = 1024;
	static int const MAX_DEPTH = 128;

	// only called from single thread - so can use a shared buffer
	static char s_fileName[MAX_FILENAME_LENGTH];
	static CallstackLine s_callstackBuffer[MAX_DEPTH];

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static bool Init( );
	static void Destroy( );
	static void Free( Callstack * cs );
	static Callstack * Allocate( unsigned int skip_frames );
	static CallstackLine * GetLines( Callstack * cs );
	static CallstackLine & GetTopLine( Callstack * cs );
};

#endif 
