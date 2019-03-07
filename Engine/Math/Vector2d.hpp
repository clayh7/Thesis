#pragma once


//-------------------------------------------------------------------------------------------------
class Vector2d
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Vector2d const ZERO;
	static Vector2d const ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	double x;
	double y;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector2d( );
	Vector2d( double setValues );
	Vector2d( double setX, double setY );
	Vector2d( Vector2d const & setVector );
	Vector2d const operator+( Vector2d const & add ) const;

	void operator+=( Vector2d const & add );
	Vector2d const operator-( ) const;
	Vector2d const operator-( Vector2d const & subtract ) const;
	void operator-=( Vector2d const & subtract );
	Vector2d const operator*( double rhs ) const;
	friend Vector2d const operator*( double lhs, Vector2d const & rhs );
	void operator*=( double rhs );
	void SetXY( double newX, double newY );
	void Normalize( );
	double Length( ) const;
	double SquareLength( ) const;
};