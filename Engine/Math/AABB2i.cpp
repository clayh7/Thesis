#include "Engine/Math/AABB2i.hpp"

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const AABB2i AABB2i::ZERO_TO_ZERO( Vector2i::ZERO, Vector2i::ZERO );
STATIC const AABB2i AABB2i::ZERO_TO_ONE( Vector2i::ZERO, Vector2i::ONE );


//-------------------------------------------------------------------------------------------------
AABB2i::AABB2i( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
AABB2i::AABB2i( Vector2i const & setMins, Vector2i const & setMaxs )
	: mins( setMins )
	, maxs( setMaxs )
{
}


//-------------------------------------------------------------------------------------------------
AABB2i::~AABB2i( )
{
}