#include "Engine/Math/AABB2f.hpp"

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC AABB2f const AABB2f::ZERO_TO_ZERO(Vector2f::ZERO, Vector2f::ZERO);
STATIC AABB2f const AABB2f::ZERO_TO_ONE( Vector2f::ZERO, Vector2f::ONE );


//-------------------------------------------------------------------------------------------------
AABB2f::AABB2f( Vector2f const & setMins, Vector2f const & setMaxs )
	: mins( setMins )
	, maxs( setMaxs )
{

}


//-------------------------------------------------------------------------------------------------
AABB2f::~AABB2f( )
{

}


//-------------------------------------------------------------------------------------------------
AABB2f const operator*( float lhs, AABB2f const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
AABB2f const AABB2f::operator*( float rhs ) const
{
	return AABB2f( mins * rhs, maxs * rhs );
}


//-------------------------------------------------------------------------------------------------
void AABB2f::operator*=( float rhs )
{
	mins *= rhs;
	maxs *= rhs;
}


//-------------------------------------------------------------------------------------------------
bool AABB2f::Contains( Vector2f const & check ) const
{
	bool xInside = check.x >= mins.x && check.x <= maxs.x;
	bool yInside = check.y >= mins.y && check.y <= maxs.y;
	return xInside && yInside;
}
