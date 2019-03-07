#include <stdarg.h>
#include <algorithm>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/Command.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/DebugSystem/Logger.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/Threads/Thread.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
const int LOGPRINTF_STACK_LOCAL_TEMP_LENGTH = 2048;
Logger * LoggingSystem = nullptr;


//-------------------------------------------------------------------------------------------------
void LogLevelCommand( Command const & command )
{
	int level = command.GetArg( 0, (int)WarningLevel_EVERY_FRAME );
	LoggingSystem->SetLogLevel( level );
	g_ConsoleSystem->AddLog( Stringf( "Log Level Set: %d", level ), Color::GREEN );
}


//-------------------------------------------------------------------------------------------------
void DebugLevelCommand( Command const & command )
{
	int level = command.GetArg( 0, (int)WarningLevel_EVERY_FRAME );
	LoggingSystem->SetDebugLevel( level );
	g_ConsoleSystem->AddLog( Stringf( "Log Level Set: %d", level ), Color::GREEN );
}


//-------------------------------------------------------------------------------------------------
void HandleMessage( LogMessage * message )
{
	if( message->warningLevel <= LoggingSystem->m_logLevelThreshold )
	{
		fwrite( message->messageString, sizeof( char ), message->messageSize, LoggingSystem->GetFileHandle( ) );
	}
	if( message->warningLevel <= LoggingSystem->m_debugLevelThreshold )
	{
		DebuggerPrintf( message->messageString );
	}
}


//-------------------------------------------------------------------------------------------------
void HandleRemainingMessages( )
{
	LogMessage * msg;
	while( LoggingSystem->m_messages->PopFront( &msg ) )
	{
		HandleMessage( msg );
		delete msg;
	}
}


//-------------------------------------------------------------------------------------------------
void LoggerThreadEntry( void * )
{
	LoggingSystem->OpenFile( );
	while( !g_isQuitting && !LoggingSystem->IsFlushReady( ) )
	{
		LogMessage * msg;
		while( LoggingSystem->m_messages->PopFront( &msg ) )
		{
			HandleMessage( msg );
			delete msg;
		}
		std::this_thread::yield( );
	}
	HandleRemainingMessages( );
	fflush( LoggingSystem->GetFileHandle( ) );
	LoggingSystem->CloseFile( );
}


//-------------------------------------------------------------------------------------------------
LogMessage::LogMessage( char const * message, size_t const & size, size_t const & level )
	: messageString( message )
	, messageSize( size )
	, warningLevel( level )
{
}


//-------------------------------------------------------------------------------------------------
LogMessage::~LogMessage( )
{
	delete messageString;
	messageString = nullptr;
}


//-------------------------------------------------------------------------------------------------
Logger::Logger( )
	: m_flushLogger( false )
	, m_loggerThread( nullptr )
	, m_fileHandle( NULL )
	, m_messages( nullptr )
	, m_logLevelThreshold( WarningLevel_EVERY_FRAME )
	, m_debugLevelThreshold( WarningLevel_EVERY_FRAME )
{
	DateTime now = Time::GetNow( );
	m_logFilename = Stringf( "Data/Logs/%.4d%.2d%.2d_%.2d%.2d%.2d.Log.txt", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec );
	m_messages = new BQueue<LogMessage*>( );

	g_ConsoleSystem->RegisterCommand( "log_level", LogLevelCommand, " [num] : Print all warning levels num or lower to log file." );
	g_ConsoleSystem->RegisterCommand( "debug_level", DebugLevelCommand, " [num] : Print all warning levels num or lower to debug output window." );

#ifdef LOG_WARNING_LEVEL
	m_logLevelThreshold = LOG_WARNING_LEVEL;
#endif // LOG_WARNING_LEVEL

#ifdef DEBUG_WARNING_LEVEL
	m_debugLevelThreshold = DEBUG_WARNING_LEVEL;
#endif // DEBUG_WARNING_LEVEL

#ifdef MAX_LOG_HISTORY
	//#TODO: Reduce the number of files down to Max Log size
	std::vector<std::string> logFiles = EnumerateFilesInFolder( "Data/Logs/", "*.Log.txt" );
	int maxLogs = MAX_LOG_HISTORY;
	if( logFiles.size( ) >= (unsigned int)maxLogs )
	{
		std::string deleteFile = logFiles.front( );
		if( remove( deleteFile.c_str() ) != 0 )
		{
			ERROR_AND_DIE( "Error deleting file" );
		}
	}
#endif // MAX_LOG_HISTORY

}


//-------------------------------------------------------------------------------------------------
Logger::~Logger( )
{
	LogFlush( );
	
	delete m_loggerThread;
	m_loggerThread = nullptr;

	//#TODO: Figure out why there is still a message in m_messages
	LogMessage * message;
	while( m_messages->PopFront( &message ) )
	{
		delete message;
		message = nullptr;
	}

	delete m_messages;
	m_messages = nullptr;
}


//-------------------------------------------------------------------------------------------------
void Logger::LogPrintfv( WarningLevel const & level, char const * format, va_list variableArgumentList )
{
	char textLiteral[LOGPRINTF_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s( textLiteral, LOGPRINTF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[LOGPRINTF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	//Add message to message queue
	DateTime now = Time::GetNow( );
	std::string timestamp = Stringf( "%.2d:%.2d:%.2d", now.tm_hour, now.tm_min, now.tm_sec );
	std::string formattedMessage = Stringf( "%s [LEVEL %u] %s\n", timestamp.c_str( ), level, textLiteral );
	if( m_messages && !m_flushLogger )
	{
		char * messageString = CreateNewCString( formattedMessage );
		LogMessage * constructedMessage = new LogMessage( messageString, formattedMessage.size( ), level );
		m_messages->PushBack( constructedMessage );
	}
}


//-------------------------------------------------------------------------------------------------
void Logger::LogPrintf( char const * format, ... )
{
	//Figure out variable argument list
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogPrintfv( WarningLevel_DEFAULT, format, variableArgumentList );
}


//-------------------------------------------------------------------------------------------------
void Logger::LogPrintf( WarningLevel const & level, char const * format, ... )
{
	//Figure out variable argument list
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogPrintfv( level, format, variableArgumentList );
}


//-------------------------------------------------------------------------------------------------
void Logger::LogPrintfWithCallstack( WarningLevel const & level, char const * format, ... )
{
	//Figure out variable argument list
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	LogPrintfv( level, format, variableArgumentList );

	//Add callstack to message queue
	Callstack * callstack = CallstackSystem::Allocate( 1 );
	CallstackLine * lines = CallstackSystem::GetLines( callstack ); //And here it is twice
	for( unsigned int index = 0; index < callstack->frame_count; ++index )
	{
		std::string callstackMesage = Stringf( "%s\n%s(%u)\n", lines[index].function_name, lines[index].filename, lines[index].line );
		char * callstackString = CreateNewCString( callstackMesage );
		LogMessage * stackLogMessage = new LogMessage( callstackString, callstackMesage.size( ), level );
		m_messages->PushBack( stackLogMessage );
	}
}


//-------------------------------------------------------------------------------------------------
void Logger::LogFlush( )
{
	if( !m_flushLogger )
	{
		m_flushLogger = true;
		m_loggerThread->Join( );
	}
}


//-------------------------------------------------------------------------------------------------
bool Logger::IsFlushReady( ) const
{
	return m_flushLogger;
}


//-------------------------------------------------------------------------------------------------
FILE * Logger::GetFileHandle( ) const
{
	return m_fileHandle;
}


//-------------------------------------------------------------------------------------------------
void Logger::Begin( )
{
	m_loggerThread = new Thread( LoggerThreadEntry );
}


//-------------------------------------------------------------------------------------------------
void Logger::OpenFile( )
{
	errno_t errorSuccess = fopen_s( &m_fileHandle, m_logFilename.c_str(), "wb" );

	//Make sure it opened something
	if( errorSuccess != 0 )
	{
		ERROR_AND_DIE( "Failed to open file" );
	}
}


//-------------------------------------------------------------------------------------------------
void Logger::CloseFile( )
{
	fclose( m_fileHandle );
}


//-------------------------------------------------------------------------------------------------
void Logger::SetLogLevel( int level )
{
	m_logLevelThreshold = level;
}


//-------------------------------------------------------------------------------------------------
void Logger::SetDebugLevel( int level )
{
	m_debugLevelThreshold = level;
}