#include "Engine/Memory/MemoryAnalytics.hpp"

#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
MemoryAnalytics * g_MemoryAnalyticsSystem = nullptr;


//#TODO: Definitely does NOT track bytes leaked correctly
#ifdef MEMORY_TRACKING
//-------------------------------------------------------------------------------------------------
void * Allocate( size_t numBytes, eMemoryTag tag )
{
	if( g_ProfilerSystem )
	{
		g_ProfilerSystem->IncrementNews( );
	}

	size_t totalSize = numBytes + sizeof( size_t ) * 2;
	if( !g_MemoryAnalyticsSystem || !g_MemoryAnalyticsSystem->m_trackMemory )
	{
		size_t * ptr = (size_t*) malloc( totalSize );
		ptr += 2;
		return ptr;
	}

	g_MemoryAnalyticsSystem->LockCallstackMap( );

	//Make room for saving the meta data
	size_t * ptr = (size_t*) malloc( totalSize );
	++g_MemoryAnalyticsSystem->m_numAllocations;
	++g_MemoryAnalyticsSystem->m_allocationsForOneSecond;
	g_MemoryAnalyticsSystem->m_totalAllocated += numBytes;
	
	//Track high-water mark
	if( g_MemoryAnalyticsSystem->m_totalAllocated > g_MemoryAnalyticsSystem->m_highestTotalAllocated )
	{
		g_MemoryAnalyticsSystem->m_highestTotalAllocated = g_MemoryAnalyticsSystem->m_totalAllocated;
	}

	//Store the meta data for the size of the data
	*ptr = (size_t) tag;
	++ptr;
	*ptr = numBytes;
	++ptr;

	//Add allocation stack to map
	auto foundAllocation = g_MemoryAnalyticsSystem->m_callstackMap.find( ptr );
	if( foundAllocation == g_MemoryAnalyticsSystem->m_callstackMap.end( ) ) // No duplicates
	{
		g_MemoryAnalyticsSystem->m_callstackMap.insert( std::pair<void*, Callstack*>( ptr, CallstackSystem::Allocate( 2 ) ) );
	}
	else
	{
		__debugbreak( );
	}
	g_MemoryAnalyticsSystem->UnlockCallstackMap( );

	return ptr;
}


//-------------------------------------------------------------------------------------------------
void * operator new ( size_t numBytes )
{
	return Allocate( numBytes, eMemoryTag_DEFAULT );
}


//-------------------------------------------------------------------------------------------------
void * operator new ( size_t numBytes, eMemoryTag tag )
{
	return Allocate( numBytes, tag );
}


//-------------------------------------------------------------------------------------------------
void * operator new []( size_t numBytes )
{
	return Allocate( numBytes, eMemoryTag_DEFAULT );
}


//-------------------------------------------------------------------------------------------------
void * operator new [] ( size_t numBytes, eMemoryTag tag )
{
	return Allocate( numBytes, tag );
}


//-------------------------------------------------------------------------------------------------
void Deallocate( void * ptr )
{
	if( g_ProfilerSystem )
	{
		g_ProfilerSystem->IncrementDeletes( );
	}
	
	if( !g_MemoryAnalyticsSystem || !g_MemoryAnalyticsSystem->m_trackMemory )
	{
		size_t * ptrStart = (size_t *) ptr;
		ptrStart -= 2;
		free( ptrStart );
		return;
	}

	g_MemoryAnalyticsSystem->LockCallstackMap( );

	//Move pointer backwards to get the position of the store data size
	size_t * metaData = (size_t *) ptr;
	--metaData;
	size_t allocationAmount = *metaData;
	--metaData;
	//eMemoryTag tag = (eMemoryTag) ( *metaData );
	//metaData is now at the start of the data and ready to be freed

	//Update allocation trackers
	if( g_MemoryAnalyticsSystem->m_numAllocations == 0 )
	{
		//Too many deallocations
		__debugbreak( );
	}
	else
	{
		--g_MemoryAnalyticsSystem->m_numAllocations;
		++g_MemoryAnalyticsSystem->m_deallocationsForOneSecond;
		g_MemoryAnalyticsSystem->m_totalAllocated -= allocationAmount;
	}

	auto foundAllocation = g_MemoryAnalyticsSystem->m_callstackMap.find( ptr );
	if( foundAllocation != g_MemoryAnalyticsSystem->m_callstackMap.end( ) )
	{
		CallstackSystem::Free( foundAllocation->second );
		g_MemoryAnalyticsSystem->m_callstackMap.erase( foundAllocation );
	}
	else
	{
		//#TODO: Problem with deleting LoggMessage* from the Logger thread
		//Doesn't exist in map and trying to remove it
		++g_MemoryAnalyticsSystem->m_numAllocations;
		//__debugbreak( );
	}
	g_MemoryAnalyticsSystem->UnlockCallstackMap( );

	//Free the pointer 
	free( metaData );
}


//-------------------------------------------------------------------------------------------------
void operator delete ( void * ptr )
{
	Deallocate( ptr );
}


//-------------------------------------------------------------------------------------------------
void operator delete [] ( void * ptr )
{
	Deallocate( ptr );
}
#endif


//-------------------------------------------------------------------------------------------------
MemoryAnalytics::MemoryAnalytics( )
	: m_trackMemory( false )
	, m_numAllocations( 0 )
	, m_totalAllocated( 0 )
	, m_highestTotalAllocated( 0 )
	, m_timeStampOfPreviousAnalysis( 0.f )
	, m_allocationsForOneSecond( 0 )
	, m_allocationsInTheLastSecond( 0 )
	, m_averageAllocationsPerSecond( 0 )
	, m_deallocationsForOneSecond( 0 )
	, m_deallocationsInTheLastSecond( 0 )
	, m_averageDeallocationsPerSecond( 0 )
{
	MemoryAnalyticsStartUp( );
}


//-------------------------------------------------------------------------------------------------
MemoryAnalytics::~MemoryAnalytics( )
{
	CleanUpCallstackStats( );
	MemoryAnalyticsShutDown( );
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::Update( )
{
	//Run once every second
	float elapsedTime = Time::TOTAL_SECONDS - m_timeStampOfPreviousAnalysis;
	if( elapsedTime >= 1.f )
	{
		m_timeStampOfPreviousAnalysis = Time::TOTAL_SECONDS;
		m_allocationsInTheLastSecond = m_allocationsForOneSecond;
		m_averageAllocationsPerSecond = (float) m_allocationsInTheLastSecond / elapsedTime;
		m_deallocationsInTheLastSecond = m_deallocationsForOneSecond;
		m_averageDeallocationsPerSecond = (float) m_deallocationsInTheLastSecond / elapsedTime;
		m_allocationsForOneSecond = 0;
		m_deallocationsForOneSecond = 0;
#if MEMORY_TRACKING == 1
		PopulateCallstackStats( );
#endif // MEMORY_TRACKING == 1
	}
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::MemoryAnalyticsStartUp( )
{
#ifdef MEMORY_TRACKING
	m_trackMemory = true; //If it wasn't already tracking memory, start tracking now
	m_startupAllocations = m_numAllocations;
	CallstackSystem::Init( );
	DebuggerPrintf( "\n//=============================================================================================\n" );
	DebuggerPrintf( "Before Start Up \n" );
	DebuggerPrintf( "Allocations: %u \n", m_numAllocations );
	DebuggerPrintf( "Bytes Allocated: %u \n", m_totalAllocated );
	DebuggerPrintf( "//=============================================================================================\n\n" );
#endif
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::MemoryAnalyticsShutDown( )
{
#ifdef MEMORY_TRACKING
	m_trackMemory = false;
#if (MEMORY_TRACKING == 1)
	Flush( );
#endif
	if( m_startupAllocations != m_numAllocations )
	{
		//ASSERT_RECOVERABLE( false, "Memory Leaks" );
	}
	DebuggerPrintf( "\n//=============================================================================================\n" );
	DebuggerPrintf( "Shut Down \n" );
	DebuggerPrintf( "Leaks: %u \n", m_numAllocations );
	DebuggerPrintf( "Bytes Leaked: %u \n", m_totalAllocated );
	DebuggerPrintf( "//=============================================================================================\n\n" );
	CallstackSystem::Destroy( );
#endif
}


//-------------------------------------------------------------------------------------------------
std::string MemoryAnalytics::GetMemoryAllocationsString( ) const
{
#ifdef MEMORY_TRACKING
	return Stringf( "Allocations: %u | Bytes Allocated: %u | Most Bytes: %u",
		m_numAllocations,
		m_totalAllocated,
		m_highestTotalAllocated
	);
#else
	return "No memory debug tracking.";
#endif
}


//-------------------------------------------------------------------------------------------------
std::string MemoryAnalytics::GetMemoryAveragesString( ) const
{
#ifdef MEMORY_TRACKING
	return Stringf( "Average Allocations: %.1f | Average Deallocations: %.1f | Allocation Rate: %.1f",
		m_averageAllocationsPerSecond,
		m_averageDeallocationsPerSecond,
		( m_averageAllocationsPerSecond - m_averageDeallocationsPerSecond )
	);
#else
	return " ";
#endif
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::Flush( )
{
	PopulateCallstackStats( ); //Calls CallstackGetLines()
	for( auto callstackStatsItem : m_callstackStatsMap )
	{
		DebuggerPrintf( "\n//---------------------------------------------------------------------------------------------\n\n" );
		CallstackStats & foundStats = callstackStatsItem.second;
		size_t leakedAllocations = foundStats.totalAllocations;
		size_t leakedBytes = foundStats.totalBytes;
		Callstack * currentCallstack = foundStats.callstackPtr;
		CallstackLine * lines = CallstackSystem::GetLines( currentCallstack ); //And here it is twice
		DebuggerPrintf( Stringf( "Allocations: %u | Bytes: %u\n", leakedAllocations, leakedBytes ).c_str( ) );
		for( unsigned int index = 0; index < currentCallstack->frame_count; ++index )
		{
			DebuggerPrintf( Stringf( "%s\n%s(%u)\n", lines[index].function_name, lines[index].filename, lines[index].line ).c_str( ) );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::PopulateCallstackStats( )
{
	CleanUpCallstackStats( );

	for( auto callstackItem : m_callstackMap )
	{
		g_MemoryAnalyticsSystem->LockCallstackMap( );
		size_t * leakedPtr = (size_t*) ( callstackItem.first );
		--leakedPtr;
		size_t leakedBytes = *leakedPtr;
		Callstack * currentCallstack = callstackItem.second;
		g_MemoryAnalyticsSystem->UnlockCallstackMap( );

		//Hash allocation location
		uint32_t callstackHash = HashMemory( currentCallstack->frameDataPtr, currentCallstack->frame_count * sizeof(void *) );

		//Find associated allocation location's allocation stats
		auto callstackStatsItem = m_callstackStatsMap.find( callstackHash );
		//Add it to map
		if( callstackStatsItem == m_callstackStatsMap.end() )
		{
			CallstackStats newStats;
			newStats.callstackPtr = currentCallstack;
			newStats.totalAllocations = 1;
			newStats.totalBytes = leakedBytes;
			CallstackLine & topLine = CallstackSystem::GetTopLine( currentCallstack );
			std::string debugMemoryLine = Stringf( "%s(%u)", topLine.filename, topLine.line );
			newStats.lineAndNumber = CreateNewCString( debugMemoryLine );
			m_callstackStatsMap.insert( std::pair<uint32_t, CallstackStats>( callstackHash, newStats ) );
		}
		//Update item in map
		else
		{
			CallstackStats & foundStats = callstackStatsItem->second;
			foundStats.totalAllocations += 1;
			foundStats.totalBytes += leakedBytes;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::CleanUpCallstackStats( )
{
	for( auto callstackStatsItem : m_callstackStatsMap )
	{
		delete callstackStatsItem.second.lineAndNumber;
		callstackStatsItem.second.lineAndNumber = nullptr;
	}
	m_callstackStatsMap.clear( );
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::LockCallstackMap( )
{
	criticalSection_callstackMap.Lock( );
}


//-------------------------------------------------------------------------------------------------
void MemoryAnalytics::UnlockCallstackMap( )
{
	criticalSection_callstackMap.Unlock( );
}
