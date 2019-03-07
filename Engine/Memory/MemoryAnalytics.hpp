#pragma once

#undef max //Needed for max() function from limits
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include "Engine/Memory/Callstack.hpp"
#include "Engine/Threads/CriticalSection.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
// Handles static STL containers
//
// Example Usage
// static std::vector<int, UntrackedAllocator<int>> gTestVector;
//-------------------------------------------------------------------------------------------------
template <typename T>
class UntrackedAllocator
{
//-------------------------------------------------------------------------------------------------
// Typedefs
//-------------------------------------------------------------------------------------------------
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;

//-------------------------------------------------------------------------------------------------
// Rebinding
//-------------------------------------------------------------------------------------------------
public:
	// convert an allocator<T> to allocator<U>
	template<typename U>
	struct rebind
	{
		typedef UntrackedAllocator<U> other;
	};

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	inline explicit UntrackedAllocator( )
	{
	}

	inline ~UntrackedAllocator( )
	{
	}

	inline explicit UntrackedAllocator( UntrackedAllocator const & )
	{
	}

	template<typename U>
	inline explicit UntrackedAllocator( UntrackedAllocator<U> const & )
	{

	}

	// address
	inline pointer address( reference r )
	{
		return &r;
	}

	inline const_pointer address( const_reference r )
	{
		return &r;
	}

	// memory allocation
	inline pointer allocate( size_type cnt, typename std::allocator<void>::const_pointer = 0 )
	{
		T *ptr = (T*) malloc( cnt * sizeof( T ) );
		return ptr;
	}

	inline void deallocate( pointer p, size_type cnt )
	{
		UNREFERENCED( cnt );
		free( p );
	}

	// size
	inline size_type max_size( ) const
	{
		return std::numeric_limits<size_type>::max( ) / sizeof( T );
	}

	// construction/destruction
	inline void construct( pointer p, T const & t )
	{
		new( p ) T( t );
	}

	inline void destroy( pointer p )
	{
		UNREFERENCED( p );
		p->~T( );
	}

	inline bool operator==( UntrackedAllocator const & a )
	{
		return this == &a;
	}

	inline bool operator!=( UntrackedAllocator const & a )
	{
		return !operator==( a );
	}
};


//-------------------------------------------------------------------------------------------------
class CallstackStats
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	size_t totalAllocations;
	size_t totalBytes;
	char const * lineAndNumber;
	Callstack * callstackPtr;
};


//-------------------------------------------------------------------------------------------------
enum eMemoryTag
{
	eMemoryTag_DEFAULT,
	eMemoryTag_UPDATE_ENGINE,
	eMemoryTag_UPDATE_GAME,
	eMemoryTag_RENDER_ENGINE,
	eMemoryTag_RENDER_GAME,
};


//-------------------------------------------------------------------------------------------------
class MemoryAnalytics;
typedef std::map<void*, Callstack*, std::less<void*>, UntrackedAllocator<std::pair<void*, Callstack*>>> UntrackedCallstackMap;
typedef std::map<uint32_t, CallstackStats, std::less<uint32_t>, UntrackedAllocator<std::pair<uint32_t, CallstackStats>>> UntrackedCallstackStatsMap;


//-------------------------------------------------------------------------------------------------
extern MemoryAnalytics* g_MemoryAnalyticsSystem;


//-------------------------------------------------------------------------------------------------
class MemoryAnalytics
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	bool m_trackMemory;
	size_t m_startupAllocations;
	size_t m_numAllocations;
	size_t m_totalAllocated;
	size_t m_highestTotalAllocated;

	float m_timeStampOfPreviousAnalysis;
	size_t m_allocationsForOneSecond;
	size_t m_allocationsInTheLastSecond;
	float m_averageAllocationsPerSecond;
	size_t m_deallocationsForOneSecond;
	size_t m_deallocationsInTheLastSecond;
	float m_averageDeallocationsPerSecond;

	UntrackedCallstackMap m_callstackMap;
	UntrackedCallstackStatsMap m_callstackStatsMap;
	CriticalSection criticalSection_callstackMap;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	MemoryAnalytics( );
	~MemoryAnalytics( );
	
	void Update( );

	void MemoryAnalyticsStartUp( );
	void MemoryAnalyticsShutDown( );
	std::string GetMemoryAllocationsString( ) const;
	std::string GetMemoryAveragesString( ) const;
	void Flush( );
	void PopulateCallstackStats( );
	void CleanUpCallstackStats( );
	void LockCallstackMap( );
	void UnlockCallstackMap( );
};