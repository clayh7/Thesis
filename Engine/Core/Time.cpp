//-----------------------------------------------------------------------------------------------
#include "Engine/Core/Time.hpp"

#include <ctime>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC float Time::DELTA_SECONDS = 0.f;
STATIC float Time::TOTAL_SECONDS = 0.f;


//-------------------------------------------------------------------------------------------------
STATIC std::vector<Clock*, UntrackedAllocator<Clock*>> Clock::s_baseClocks;


//-------------------------------------------------------------------------------------------------
DateTime Time::GetNow( )
{
	time_t t = time( 0 );   // get time now
	DateTime result;
	localtime_s( &result, &t );
	return result;
}


//-------------------------------------------------------------------------------------------------
double InitializeTime( LARGE_INTEGER& out_initialTime )
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );
	return( 1.0 / static_cast<double>( countsPerSecond.QuadPart ) );
}


//-----------------------------------------------------------------------------------------------
// A simple high-precision time utility function for Windows
// based on code by Squirrel Eiserloh
double Time::GetCurrentTimeSeconds( )
{
	static LARGE_INTEGER initialTime;
	static double secondsPerCount = InitializeTime( initialTime );
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter( &currentCount );
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - initialTime.QuadPart;

	double currentSeconds = static_cast<double>( elapsedCountsSinceInitialTime ) * secondsPerCount;
	return currentSeconds;
}


//-------------------------------------------------------------------------------------------------
LARGE_INTEGER GetOpCount( )
{
	LARGE_INTEGER opCount;
	QueryPerformanceCounter( &opCount );
	return opCount;
}


//-------------------------------------------------------------------------------------------------
uint64_t Time::GetCurrentOpCount( )
{
	static LARGE_INTEGER initialOpCount = GetOpCount( );
	LARGE_INTEGER currentOpCount = GetOpCount( );
	LONGLONG elapsedOpCount = currentOpCount.QuadPart - initialOpCount.QuadPart;
	return (uint64_t) elapsedOpCount;
}


//-------------------------------------------------------------------------------------------------
double GetSecondsPerOp( )
{
	LARGE_INTEGER opPerSecond;
	QueryPerformanceFrequency( &opPerSecond );
	return( 1.0 / static_cast<double>( opPerSecond.QuadPart ) );
}


//-------------------------------------------------------------------------------------------------
double Time::GetTimeFromOpCount( uint64_t opCount )
{
	static double secondsPerOp = GetSecondsPerOp( );
	return static_cast<double>( opCount ) * secondsPerOp;
}


//-------------------------------------------------------------------------------------------------
StopWatch::StopWatch( std::string const & name /*= "StopWatch"*/ )
	: stopWatchName( name )
	, startTime( ( float ) Time::GetCurrentTimeSeconds( ) )
	, lapTime( startTime )
{
}


//-------------------------------------------------------------------------------------------------
void StopWatch::PrintLap( std::string const & label /*= "" */ )
{
	float currentTime = ( float ) Time::GetCurrentTimeSeconds( );
	float elapsedTime = currentTime - lapTime;
	lapTime = currentTime;
	if ( strcmp( label.c_str( ), "" ) == 0 )
	{
		g_ConsoleSystem->AddLog( Stringf( "%s: %.6f", stopWatchName.c_str( ), elapsedTime ), Color::WHITE );
	}
	else
	{
		g_ConsoleSystem->AddLog( Stringf( "%s: %.6f", label.c_str( ), elapsedTime ), Color::WHITE );
	}
}


//-------------------------------------------------------------------------------------------------
void StopWatch::PrintTime( std::string const & label /*= "" */ )
{
	lapTime = ( float ) Time::GetCurrentTimeSeconds( );
	float elapsedTime = lapTime - startTime;
	if ( strcmp( label.c_str( ), "" ) == 0 )
	{
		g_ConsoleSystem->AddLog( Stringf( "%s: %.6f", stopWatchName.c_str( ), elapsedTime ), Color::WHITE );
	}
	else
	{
		g_ConsoleSystem->AddLog( Stringf( "%s: %.6f", label.c_str( ), elapsedTime ), Color::WHITE );
	}
}


//-------------------------------------------------------------------------------------------------
void StopWatch::Reset( )
{
	startTime = ( float ) Time::GetCurrentTimeSeconds( );
}


//-------------------------------------------------------------------------------------------------
void Clock::DestroyClocks( )
{
	for( Clock * child : s_baseClocks )
	{
		delete child;
		child = nullptr;
	}
	s_baseClocks.clear( );
}


//-------------------------------------------------------------------------------------------------
Clock::~Clock( )
{
	for( Clock * child : children )
	{
		delete child;
		child = nullptr;
	}
	children.clear( );
}


//-------------------------------------------------------------------------------------------------
Clock::Clock( Clock * parentClock /*= nullptr */ )
	: deltaSeconds( 0.f )
	, currentSeconds( 0.f )
	, scale( 0.f )
	, paused( false )
{
	if( parentClock )
	{
		parentClock->children.push_back( this );
	}
	else
	{
		s_baseClocks.push_back( this );
	}
}


//-------------------------------------------------------------------------------------------------
void Clock::Update( float dt )
{
	if( paused )
	{
		dt = 0;
	}
	dt *= scale;
	deltaSeconds = dt;
	currentSeconds += dt;

	for( Clock * child : children )
	{
		child->Update( dt );
	}
}