#pragma once


//-------------------------------------------------------------------------------------------------
typedef unsigned long long uint64_t;


//-------------------------------------------------------------------------------------------------
class BProfilerSample
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	char const * tag;
	uint64_t opCountStart;
	uint64_t opCountEnd;
	size_t newCount;
	size_t deleteCount;
	BProfilerSample * parent;
	BProfilerSample * next;
	BProfilerSample * children;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	BProfilerSample( );
	~BProfilerSample( );

	void SetTag( char const * setTag );
	void AddChild( BProfilerSample * child );
};