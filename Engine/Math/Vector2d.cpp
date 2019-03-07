#include "Engine/Math/Vector2d.hpp"

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector2d Vector2d::ZERO( 0 );
STATIC const Vector2d Vector2d::ONE( 1 );


//-------------------------------------------------------------------------------------------------
Vector2d::Vector2d( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector2d const operator*( double lhs, Vector2d const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
Vector2d::Vector2d( double setValues )
	: x( setValues )
	, y( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2d::Vector2d( Vector2d const &setVector )
	: x( setVector.x )
	, y( setVector.y )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2d::Vector2d( double setX, double setY )
	: x( setX )
	, y( setY )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2d const Vector2d::operator+( Vector2d const &add ) const
{
	return Vector2d( x + add.x, y + add.y );
}


//-------------------------------------------------------------------------------------------------
void Vector2d::operator+=( Vector2d const &add )
{
	x += add.x;
	y += add.y;
}


//-------------------------------------------------------------------------------------------------
Vector2d const Vector2d::operator-( Vector2d const &subtract ) const
{
	return Vector2d( x - subtract.x, y - subtract.y );
}


//-------------------------------------------------------------------------------------------------
void Vector2d::operator-=( Vector2d const &subtract )
{
	x -= subtract.x;
	y -= subtract.y;
}


//-------------------------------------------------------------------------------------------------
Vector2d const Vector2d::operator*( double rhs ) const
{
	return Vector2d( x * rhs, y * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector2d const Vector2d::operator-( ) const
{
	return Vector2d( -x, -y );
}


//-------------------------------------------------------------------------------------------------
void Vector2d::operator*=( double rhs )
{
	x *= rhs;
	y *= rhs;
}


//-------------------------------------------------------------------------------------------------
void Vector2d::SetXY( double setX, double setY )
{
	x = setX;
	y = setY;
}


//-------------------------------------------------------------------------------------------------
void Vector2d::Normalize( )
{
	double length = Length( );
	double oneOverLength = 1.f / length;
	x *= oneOverLength;
	y *= oneOverLength;
}


//-------------------------------------------------------------------------------------------------

double Vector2d::Length( ) const
{
	return sqrt( ( double ) SquareLength( ) );
}


//-------------------------------------------------------------------------------------------------
double Vector2d::SquareLength( ) const
{
	return x*x + y*y;
}