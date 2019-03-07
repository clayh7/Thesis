#include "Engine/Math/Vector3d.hpp"

#include "Engine/Math/Vector2d.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector3d Vector3d::ZERO( 0.f );
STATIC const Vector3d Vector3d::ONE( 1.f );


//-------------------------------------------------------------------------------------------------
Vector3d::Vector3d( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector3d::Vector3d( double setValues )
	: x( setValues )
	, y( setValues )
	, z( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3d::Vector3d( Vector3d const & setVector )
	: x( setVector.x )
	, y( setVector.y )
	, z( setVector.z )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3d::Vector3d( Vector2d const & setXY, double setZ )
	: x( setXY.x )
	, y( setXY.y )
	, z( setZ )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3d::Vector3d( double setX, Vector2d const & setYZ )
	: x( setX )
	, y( setYZ.x )
	, z( setYZ.y )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3d::Vector3d( double setX, double setY, double setZ )
	: x( setX )
	, y( setY )
	, z( setZ )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3d const Vector3d::operator+( Vector3d const & add ) const
{
	return Vector3d( x + add.x, y + add.y, z + add.z );
}


//-------------------------------------------------------------------------------------------------
void Vector3d::operator+=( Vector3d const & add )
{
	x += add.x;
	y += add.y;
	z += add.z;
}


//-------------------------------------------------------------------------------------------------
Vector3d const Vector3d::operator-( ) const
{
	return Vector3d( -x, -y, -z );
}


//-------------------------------------------------------------------------------------------------
Vector3d const Vector3d::operator-( Vector3d const & subtract ) const
{
	return Vector3d( x - subtract.x, y - subtract.y, z - subtract.z );
}


//-------------------------------------------------------------------------------------------------
void Vector3d::operator-=( Vector3d const & subtract )
{
	x -= subtract.x;
	y -= subtract.y;
	z -= subtract.z;
}


//-------------------------------------------------------------------------------------------------
Vector3d const Vector3d::operator*( double rhs ) const
{
	return Vector3d( x * rhs, y * rhs, z * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector3d const operator*( double lhs, Vector3d const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
void Vector3d::operator*=( double rhs )
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
}


//-------------------------------------------------------------------------------------------------
bool Vector3d::operator==( Vector3d const & check ) const
{
	return ( x == check.x ) && ( y == check.y ) && ( z == check.z );
}


//-------------------------------------------------------------------------------------------------
void Vector3d::SetXYZ( double setX, double setY, double setZ )
{
	x = setX;
	y = setY;
	z = setZ;
}


//-------------------------------------------------------------------------------------------------
void Vector3d::Normalize( )
{
	double length = Length( );
	if ( length == 0.f )
		return;
	double oneOverLength = 1.f / length;
	x *= oneOverLength;
	y *= oneOverLength;
	z *= oneOverLength;
}


//-------------------------------------------------------------------------------------------------
double Vector3d::Length( ) const
{
	return sqrt( ( double ) SquareLength( ) );
}


//-------------------------------------------------------------------------------------------------
double Vector3d::SquareLength( ) const
{
	return x*x + y*y + z*z;
}


//-------------------------------------------------------------------------------------------------
Vector2d Vector3d::XY( )
{
	return Vector2d( x, y );
}