#pragma once

#include "Engine/Math/Vector2f.hpp"


//-------------------------------------------------------------------------------------------------
class AABB2f
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static AABB2f const ZERO_TO_ZERO;
	static AABB2f const ZERO_TO_ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector2f mins;
	Vector2f maxs;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	AABB2f( Vector2f const & setMins, Vector2f const & setMaxs );
	~AABB2f( );
	AABB2f const operator*( float rhs ) const;
	friend AABB2f const operator*( float lhs, AABB2f const & rhs );
	void operator*=( float rhs );
	bool Contains( Vector2f const & check ) const;
};