#pragma once
#include "Engine/Threads/BQueue.hpp"


//-------------------------------------------------------------------------------------------------
enum WarningLevel
{
	WarningLevel_NONE,
	WarningLevel_SEVERE,
	WarningLevel_ASSERT_RECOVERABLE,
	WarningLevel_DEFAULT,
	WarningLevel_EVERY_FRAME,
};


//-------------------------------------------------------------------------------------------------
class LogMessage
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	char const * messageString;
	size_t messageSize;
	size_t warningLevel;
	//#TODO: Add callstack ptr* so that 
	//LogTag tag;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	LogMessage( char const * message, size_t const & size, size_t const & level );
	~LogMessage( );
};


//-------------------------------------------------------------------------------------------------
class Logger;
class Thread;


//-------------------------------------------------------------------------------------------------
extern Logger * LoggingSystem;


//-------------------------------------------------------------------------------------------------
class Logger
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	bool m_flushLogger;
	std::string m_logFilename;
	Thread * m_loggerThread;
	FILE * m_fileHandle;

public:
	BQueue<LogMessage*> * m_messages;
	size_t m_logLevelThreshold;
	size_t m_debugLevelThreshold;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Logger( );
	~Logger( );

public:
	void LogPrintfv( WarningLevel const & level, char const * format, va_list variableArgumentList );
	void LogPrintf( char const * format, ... );
	void LogPrintf( WarningLevel const & level, char const * format, ... );
	void LogPrintfWithCallstack( WarningLevel const & level, char const * format, ... );
	void LogFlush( );
	bool IsFlushReady( ) const;
	FILE * GetFileHandle( ) const;
	void Begin( );
	void OpenFile( );
	void CloseFile( );
	void SetLogLevel( int level );
	void SetDebugLevel( int level );
};