#include "Engine/DebugSystem/BProfilerReport.hpp"


//-------------------------------------------------------------------------------------------------
BProfilerReport::BProfilerReport( )
	: tag( nullptr )
	, index( 0 )
	, depth( 0 )
	, newCount( 0 )
	, deleteCount( 0 )
	, totalTime( 0 )
	, childrenTime( 0 )
	, selfTime( 0 )
	, percent( 0.f )
{
	//Nothing
}