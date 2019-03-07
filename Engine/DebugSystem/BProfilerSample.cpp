#include "Engine/DebugSystem/BProfilerSample.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"


//-------------------------------------------------------------------------------------------------
BProfilerSample::BProfilerSample( )
	: tag( nullptr )
	, opCountStart( Time::GetCurrentOpCount( ) )
	, opCountEnd( 0 )
	, newCount( 0 )
	, deleteCount( 0 )
	, parent( nullptr )
	, next( nullptr )
	, children( nullptr )
{
}


//-------------------------------------------------------------------------------------------------
BProfilerSample::~BProfilerSample( )
{
	while( children )
	{
		BProfilerSample * deleteSample = children;
		children = children->next;
		g_ProfilerSystem->Delete( deleteSample );
	}
}


//-------------------------------------------------------------------------------------------------
void BProfilerSample::SetTag( char const * setTag )
{
	tag = setTag;
}


//-------------------------------------------------------------------------------------------------
void BProfilerSample::AddChild( BProfilerSample * child )
{
	child->next = children;
	child->parent = this;
	children = child;
}