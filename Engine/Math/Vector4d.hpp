#pragma once


//-------------------------------------------------------------------------------------------------
class Vector3d;


//-------------------------------------------------------------------------------------------------
class Vector4d
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Vector4d const ZERO;
	static Vector4d const ONE;

	//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	double x;
	double y;
	double z;
	double w;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector4d( );
	Vector4d( double setValues );
	Vector4d( double setX, double setY, double setZ, double setW );
	Vector4d( Vector3d const & setXYZ, double setW );
	Vector4d( double setX, Vector3d const & setYZW );
	Vector4d( Vector4d const & setVector );

	Vector4d const operator+( Vector4d const & add ) const;
	void operator+=( Vector4d const & add );
	Vector4d const operator-( ) const;
	Vector4d const operator-( Vector4d const & subtract ) const;
	void operator-=( Vector4d const & subtract );
	Vector4d const operator*( double rhs ) const;
	friend Vector4d const operator*( double lhs, Vector4d const & rhs );
	void operator*=( double rhs );
	bool Vector4d::operator==( Vector4d const & check ) const;
	void SetXYZW( double newX, double newY, double newZ, double newW );
	void Normalize( );
	double Length( ) const;
	double SquareLength( ) const;
	Vector3d XYZ( );
};