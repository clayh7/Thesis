#include "Engine/DebugSystem/DebugLog.hpp"

#include <stdarg.h>
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
const int PRINTF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-------------------------------------------------------------------------------------------------
DebugLog::DebugLog( std::string filename )
	: m_opened( true )
{
	errno_t errorSuccess = fopen_s( &m_fileHandle, filename.c_str( ), "wb" );

	//Make sure it opened something
	if( errorSuccess != 0 )
	{
		m_opened = false;
	}
}


//-------------------------------------------------------------------------------------------------
DebugLog::~DebugLog( )
{
	if( m_opened )
	{
		fclose( m_fileHandle );
	}
}


//-------------------------------------------------------------------------------------------------
void DebugLog::Printf( char const * format, ... )
{
	//Figure out variable argument list
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	Printfv( format, variableArgumentList );
}


//-------------------------------------------------------------------------------------------------
void DebugLog::WriteToFile( )
{
	while( m_logs.size( ) > 0 && m_opened )
	{
		std::string currentLog = m_logs.front( );
		fwrite( currentLog.c_str( ), sizeof( char ), currentLog.size( ), m_fileHandle );
		m_logs.pop( );
	}
}


//-------------------------------------------------------------------------------------------------
void DebugLog::Printfv( char const * format, va_list variableArgumentList )
{
	char textLiteral[PRINTF_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s( textLiteral, PRINTF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[PRINTF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	//Add log to log queue
	DateTime now = Time::GetNow( );
	std::string timestamp = Stringf( "%.2d:%.2d:%.2d", now.tm_hour, now.tm_min, now.tm_sec );
	std::string formattedMessage = Stringf( "%s %s\n", timestamp.c_str( ), textLiteral );
	m_logs.push( formattedMessage );
}