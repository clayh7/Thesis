#pragma once


//-------------------------------------------------------------------------------------------------
class Vector4i
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const Vector4i ZERO;
	static const Vector4i ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	int x;
	int y;
	int z;
	int w;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector4i( );
	Vector4i( int setValues );
	Vector4i( int setX, int setY, int setZ, int setW );

	Vector4i const operator+( Vector4i const & add ) const;
	Vector4i const operator-( ) const;
	Vector4i const operator-( Vector4i const & subtract ) const;
	Vector4i const operator*( int rhs ) const;
	friend Vector4i const operator*( int lhs, Vector4i const & rhs );
	bool Vector4i::operator==( Vector4i const & check ) const;
	bool Vector4i::operator!=( Vector4i const & check ) const;
};