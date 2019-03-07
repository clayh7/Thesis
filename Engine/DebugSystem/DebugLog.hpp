#pragma once
#include <queue>


//-------------------------------------------------------------------------------------------------
class DebugLog
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	bool m_opened;
	FILE * m_fileHandle;
	std::queue<std::string> m_logs;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	DebugLog( std::string filename );
	~DebugLog( );

	void Printf( char const * format, ... );
	void WriteToFile( );

private:
	void Printfv( char const * format, va_list variableArgumentList );
};