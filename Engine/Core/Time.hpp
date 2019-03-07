#pragma once
#include <string>
#include <vector>
#include "Engine/Memory/MemoryAnalytics.hpp"


//-------------------------------------------------------------------------------------------------
typedef struct tm DateTime;


//-----------------------------------------------------------------------------------------------
//#TODO: Add time scale functionality
class Time
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static float DELTA_SECONDS;
	static float TOTAL_SECONDS;
	static DateTime GetNow( );
	static double GetCurrentTimeSeconds( );
	static uint64_t GetCurrentOpCount( );
	static double GetTimeFromOpCount( uint64_t opCount );
};


//-------------------------------------------------------------------------------------------------
class StopWatch
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	std::string stopWatchName;
	float startTime;
	float lapTime;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	StopWatch( std::string const & name = "StopWatch" );
	void PrintLap( std::string const & label = "" );
	void PrintTime( std::string const & label = "" );
	void Reset( );
};


//-------------------------------------------------------------------------------------------------
class Clock
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static std::vector<Clock*, UntrackedAllocator<Clock*>> s_baseClocks;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void DestroyClocks( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	float deltaSeconds;
	float currentSeconds;
	float scale;
	bool paused;
	std::vector<Clock*> children;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
private:
	~Clock( ); //I specifically want to control when clocks are destroyed (see DestroyClocks())

public:
	Clock( Clock * parentClock = nullptr );
	void Update( float dt );
};