#pragma once


//-------------------------------------------------------------------------------------------------
class Vector3f;
class Vector2f;


//-------------------------------------------------------------------------------------------------
class Vector4f
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Vector4f const ZERO;
	static Vector4f const ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	float x;
	float y;
	float z;
	float w;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector4f( );
	Vector4f( float setValues );
	Vector4f( float setX, float setY, float setZ, float setW );
	Vector4f( Vector3f const & setXYZ, float setW );
	Vector4f( Vector2f const &setXY, float setZ, float setW );
	Vector4f( float setX, Vector3f const & setYZW );
	Vector4f( Vector4f const & setVector );

	Vector4f const operator+( Vector4f const & add ) const;
	void operator+=( Vector4f const & add );
	Vector4f const operator-( ) const;
	Vector4f const operator-( Vector4f const & subtract ) const;
	void operator-=( Vector4f const & subtract );
	Vector4f const operator*( float rhs ) const;
	friend Vector4f const operator*( float lhs, Vector4f const & rhs );
	void operator*=( float rhs );
	bool Vector4f::operator==( Vector4f const & check ) const;
	void SetXYZW( float newX, float newY, float newZ, float newW );
	void Normalize( );
	float Length( ) const;
	float SquareLength( ) const;
	Vector3f XYZ( ) const;
	Vector2f XY( ) const;
};