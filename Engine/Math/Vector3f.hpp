#pragma once


//-------------------------------------------------------------------------------------------------
class Vector2f;


//-------------------------------------------------------------------------------------------------
class Vector3f
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Vector3f const ZERO;
	static Vector3f const ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	float x;
	float y;
	float z;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector3f( );
	Vector3f( float setValues );
	Vector3f( float setX, float setY, float setZ );
	Vector3f( Vector2f const & setXY, float setZ = 0.f );
	Vector3f( float setX, Vector2f const & setYZ );
	Vector3f( Vector3f const & setVector );

	Vector3f const operator+( Vector3f const & add ) const;
	void operator+=( Vector3f const & add );
	Vector3f const operator-( ) const;
	Vector3f const operator-( Vector3f const & subtract ) const;
	void operator-=( Vector3f const & subtract );
	Vector3f const operator*( float rhs ) const;
	friend Vector3f const operator*( float lhs, Vector3f const & rhs );
	void operator*=( float rhs );
	bool operator==( Vector3f const & check ) const;
	bool operator!=( Vector3f const & check ) const;
	void SetXYZ( float newX, float newY, float newZ );
	void Normalize( );
	float Length( ) const;
	float SquareLength( ) const;
	Vector2f XY( );
};