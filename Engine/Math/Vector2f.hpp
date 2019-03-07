#pragma once

#include <string>


//-------------------------------------------------------------------------------------------------
class Vector2i;


//-------------------------------------------------------------------------------------------------
class Vector2f
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Vector2f const ZERO;
	static Vector2f const ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	float x;
	float y;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector2f( );
	Vector2f( float setValues );
	Vector2f( float setX, float setY );
	Vector2f( Vector2f const & setVector );
	Vector2f( Vector2i const & setVector );
	Vector2f( std::string valuePair );

	Vector2f const operator+( Vector2f const & add ) const;
	void operator+=( Vector2f const & add );
	Vector2f const operator-() const;
	Vector2f const operator-( Vector2f const & subtract ) const;
	void operator-=( Vector2f const & subtract );
	Vector2f const operator*( Vector2f const & rhs ) const;
	friend Vector2f const operator*( float lhs, Vector2f const & rhs );
	void operator*=( float rhs );
	Vector2f const operator/( Vector2f const & rhs );
	bool Vector2f::operator==( Vector2f const & check ) const;
	bool Vector2f::operator!=( Vector2f const & check ) const;
	bool Vector2f::operator<=( Vector2f const & rhs ) const;
	void SetXY( float newX, float newY );
	Vector2f Normal( );
	void Normalize( );
	float Length( ) const;
	float SquareLength( ) const;
	void Rotate( float degrees );
};