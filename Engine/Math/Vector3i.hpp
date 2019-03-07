#pragma once


//-------------------------------------------------------------------------------------------------
class Vector3i
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const Vector3i ZERO;
	static const Vector3i ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	int x;
	int y;
	int z;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector3i( int setValues );
	Vector3i( int setX, int setY, int setZ );

	Vector3i const operator+( Vector3i const & add ) const;
	Vector3i const operator-( ) const;
	Vector3i const operator-( Vector3i const & subtract ) const;
	Vector3i const operator*( int rhs ) const;
	friend Vector3i const operator*( int lhs, Vector3i const & rhs );
	bool Vector3i::operator==( Vector3i const & check ) const;
	bool Vector3i::operator!=( Vector3i const & check ) const;
};