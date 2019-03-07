#include "Engine/Math/AABB3.hpp"

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const AABB3 AABB3::ZERO_TO_ZERO( Vector3f::ZERO, Vector3f::ZERO );
STATIC const AABB3 AABB3::ZERO_TO_ONE( Vector3f::ZERO, Vector3f::ONE );


//-------------------------------------------------------------------------------------------------
AABB3::AABB3( Vector3f const & setMins, Vector3f const & setMaxs )
: mins( setMins )
, maxs( setMaxs )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
AABB3::~AABB3( )
{
	//Nothing
}
