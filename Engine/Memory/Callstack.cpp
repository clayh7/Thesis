#include "Engine/Memory/Callstack.hpp"

#pragma  warning( disable : 4091) // Removing warning about typedef an enum

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>
#include <DbgHelp.h>
#include <map>
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
typedef BOOL( __stdcall *sym_initialize_t )( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
typedef BOOL( __stdcall *sym_cleanup_t )( IN HANDLE hProcess );
typedef BOOL( __stdcall *sym_from_addr_t )( IN HANDLE hProcess, IN DWORD64 Address, OUT PDWORD64 Displacement, OUT PSYMBOL_INFO Symbol );
typedef BOOL( __stdcall *sym_get_line_t )( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Symbol );


//-------------------------------------------------------------------------------------------------
static HMODULE g_DebugHelp;
static HANDLE g_Process;
static SYMBOL_INFO * g_Symbol;

static sym_initialize_t L_SymInitialize;
static sym_cleanup_t L_SymCleanup;
static sym_from_addr_t L_SymFromAddr;
static sym_get_line_t L_SymGetLineFromAddr64;

STATIC char CallstackSystem::s_fileName[MAX_FILENAME_LENGTH];
STATIC CallstackLine CallstackSystem::s_callstackBuffer[MAX_DEPTH];


//------------------------------------------------------------------------
bool CallstackSystem::Init( )
{
	g_DebugHelp = LoadLibraryA( "dbghelp.dll" );
	if( g_DebugHelp == nullptr )
	{
		ASSERT_RECOVERABLE( false, "DebugHelp didn't load" );
		return false;
	}
	L_SymInitialize = (sym_initialize_t) GetProcAddress( g_DebugHelp, "SymInitialize" );
	L_SymCleanup = (sym_cleanup_t) GetProcAddress( g_DebugHelp, "SymCleanup" );
	L_SymFromAddr = (sym_from_addr_t) GetProcAddress( g_DebugHelp, "SymFromAddr" );
	L_SymGetLineFromAddr64 = (sym_get_line_t) GetProcAddress( g_DebugHelp, "SymGetLineFromAddr64" );

	g_Process = GetCurrentProcess( );
	L_SymInitialize( g_Process, NULL, TRUE );

	g_Symbol = (SYMBOL_INFO *) malloc( sizeof( SYMBOL_INFO ) + ( MAX_FILENAME_LENGTH * sizeof( char ) ) );
	g_Symbol->MaxNameLen = MAX_FILENAME_LENGTH;
	g_Symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

	return true;
}


//-------------------------------------------------------------------------------------------------
void CallstackSystem::Destroy( )
{
	L_SymCleanup( g_Process );

	FreeLibrary( g_DebugHelp );
	g_DebugHelp = NULL;
}


//-------------------------------------------------------------------------------------------------
void CallstackSystem::Free( Callstack * cs )
{
	free( cs );
}


//-------------------------------------------------------------------------------------------------
Callstack * CallstackSystem::Allocate( unsigned int skip_frames )
{
	void * frameData[MAX_DEPTH];
	unsigned int frameCount = CaptureStackBackTrace( 1 + skip_frames, MAX_DEPTH, frameData, NULL );

	size_t size = sizeof( Callstack ) + sizeof( void* ) * frameCount;
	void * buffer = malloc( size );
	Callstack * cs = (Callstack*) buffer;
	cs->frameDataPtr = ( void** ) ( cs + 1 );
	cs->frame_count = frameCount;
	memcpy( cs->frameDataPtr, frameData, sizeof( void* ) * frameCount );

	return cs;
}


//-------------------------------------------------------------------------------------------------
// Should only be called from the debug trace thread.  
CallstackLine * CallstackSystem::GetLines( Callstack * cs )
{
	IMAGEHLP_LINE64 LineInfo;
	DWORD LineDisplacement = 0; // Displacement from the beginning of the line 
	LineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

	unsigned int count = cs->frame_count;
	for( unsigned int i = 0; i < count; ++i )
	{
		CallstackLine * line = &( s_callstackBuffer[i] );
		DWORD64 ptr = (DWORD64) ( cs->frameDataPtr[i] );
		L_SymFromAddr( g_Process, ptr, 0, g_Symbol );

		strncpy_s( line->function_name, g_Symbol->Name, 127 );

		BOOL bRet = L_SymGetLineFromAddr64(
			GetCurrentProcess( ),	// Process handle of the current process 
			ptr,					// Address 
			&LineDisplacement,		// Displacement will be stored here by the function 
			&LineInfo );			// File name / line information will be stored here 

		if( bRet )
		{
			line->line = LineInfo.LineNumber;
			char const * filename = LineInfo.FileName;

			strncpy_s( line->filename, filename, 127 );
			line->offset = LineDisplacement;
		}
		else
		{
			line->line = 0;
			line->offset = 0;
			strncpy_s( line->filename, "N/A", 127 );
		}
	}

	return s_callstackBuffer;
}


//-------------------------------------------------------------------------------------------------
// Specialized version to get the top callstack
CallstackLine & CallstackSystem::GetTopLine( Callstack * cs )
{
	IMAGEHLP_LINE64 LineInfo;
	DWORD LineDisplacement = 0; // Displacement from the beginning of the line 
	LineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

	CallstackLine * line = &( s_callstackBuffer[0] );
	DWORD64 ptr = (DWORD64) ( cs->frameDataPtr[0] );
	L_SymFromAddr( g_Process, ptr, 0, g_Symbol );

	strncpy_s( line->function_name, g_Symbol->Name, 127 );

	BOOL bRet = L_SymGetLineFromAddr64(
		GetCurrentProcess( ),	// Process handle of the current process 
		ptr,					// Address 
		&LineDisplacement,		// Displacement will be stored here by the function 
		&LineInfo );			// File name / line information will be stored here 

	if( bRet )
	{
		line->line = LineInfo.LineNumber;
		char const * filename = LineInfo.FileName;

		strncpy_s( line->filename, filename, 127 );
		line->offset = LineDisplacement;
	}
	else
	{
		line->line = 0;
		line->offset = 0;
		strncpy_s( line->filename, "N/A", 127 );
	}

	return s_callstackBuffer[0];
}
