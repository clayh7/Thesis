#include "Engine/DebugSystem/BProfilerScopedSample.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/Console.hpp"


//-------------------------------------------------------------------------------------------------
BScopedSample::BScopedSample( char const * sampleTag )
	: tag( sampleTag )
	, startSampleTime( Time::GetCurrentOpCount( ) )
	, stopSampleTime( 0 )
{
}


//-------------------------------------------------------------------------------------------------
BScopedSample::~BScopedSample( )
{
	stopSampleTime = Time::GetCurrentOpCount( );
	double sampleTime = Time::GetTimeFromOpCount( stopSampleTime - startSampleTime );
	g_ConsoleSystem->AddLog( Stringf( "%s: %.5fseconds", tag, sampleTime ), Console::DEFAULT );
}