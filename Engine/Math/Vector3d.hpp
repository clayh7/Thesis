#pragma once


//-------------------------------------------------------------------------------------------------
class Vector2d;


//-------------------------------------------------------------------------------------------------
class Vector3d
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Vector3d const ZERO;
	static Vector3d const ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	double x;
	double y;
	double z;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector3d( );
	Vector3d( double setValues );
	Vector3d( double setX, double setY, double setZ );
	Vector3d( Vector2d const & setXY, double setZ );
	Vector3d( double setX, Vector2d const & setYZ );
	Vector3d( Vector3d const & setVector );

	Vector3d const operator+( Vector3d const & add ) const;
	void operator+=( Vector3d const & add );
	Vector3d const operator-( ) const;
	Vector3d const operator-( Vector3d const & subtract ) const;
	void operator-=( Vector3d const & subtract );
	Vector3d const operator*( double rhs ) const;
	friend Vector3d const operator*( double lhs, Vector3d const & rhs );
	void operator*=( double rhs );
	bool operator==( Vector3d const & check ) const;
	void SetXYZ( double newX, double newY, double newZ );
	void Normalize( );
	double Length( ) const;
	double SquareLength( ) const;
	Vector2d XY( );
};