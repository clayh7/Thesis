#include "Engine/Math/Vector3f.hpp"

#include "Engine/Math/Vector2f.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector3f Vector3f::ZERO( 0.f );
STATIC const Vector3f Vector3f::ONE( 1.f );


//-------------------------------------------------------------------------------------------------
Vector3f::Vector3f( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector3f::Vector3f( float setValues )
	: x( setValues )
	, y( setValues )
	, z( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3f::Vector3f( Vector3f const & setVector )
	: x( setVector.x )
	, y( setVector.y )
	, z( setVector.z )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3f::Vector3f( Vector2f const & setXY, float setZ /*= 0.f*/ )
	: x( setXY.x )
	, y( setXY.y )
	, z( setZ )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3f::Vector3f( float setX, Vector2f const & setYZ )
	: x( setX )
	, y( setYZ.x )
	, z( setYZ.y )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3f::Vector3f( float setX, float setY, float setZ )
	: x( setX )
	, y( setY )
	, z( setZ )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3f const Vector3f::operator+( Vector3f const & add ) const
{
	return Vector3f( x + add.x, y + add.y, z + add.z );
}


//-------------------------------------------------------------------------------------------------
void Vector3f::operator+=( Vector3f const & add )
{
	x += add.x;
	y += add.y;
	z += add.z;
}


//-------------------------------------------------------------------------------------------------
Vector3f const Vector3f::operator-( ) const
{
	return Vector3f( -x, -y, -z );
}


//-------------------------------------------------------------------------------------------------
Vector3f const Vector3f::operator-( Vector3f const & subtract ) const
{
	return Vector3f( x - subtract.x, y - subtract.y, z - subtract.z );
}


//-------------------------------------------------------------------------------------------------
void Vector3f::operator-=( Vector3f const & subtract )
{
	x -= subtract.x;
	y -= subtract.y;
	z -= subtract.z;
}


//-------------------------------------------------------------------------------------------------
Vector3f const Vector3f::operator*( float rhs ) const
{
	return Vector3f( x * rhs, y * rhs, z * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector3f const operator*( float lhs, Vector3f const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
void Vector3f::operator*=( float rhs )
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
}


//-------------------------------------------------------------------------------------------------
bool Vector3f::operator==( Vector3f const & check ) const
{
	return ( x == check.x ) && ( y == check.y ) && ( z == check.z );
}


//-------------------------------------------------------------------------------------------------
bool Vector3f::operator!=( Vector3f const & check ) const
{
	return ( x != check.x ) || ( y != check.y ) || ( z != check.z );
}


//-------------------------------------------------------------------------------------------------
void Vector3f::SetXYZ( float setX, float setY, float setZ )
{
	x = setX;
	y = setY;
	z = setZ;
}


//-------------------------------------------------------------------------------------------------
void Vector3f::Normalize( )
{
	float length = Length( );
	if ( length == 0.f )
		return;
	float oneOverLength = 1.f / length;
	x *= oneOverLength;
	y *= oneOverLength;
	z *= oneOverLength;
}


//-------------------------------------------------------------------------------------------------
float Vector3f::Length( ) const
{
	return sqrt( (float)SquareLength( ) );
}


//-------------------------------------------------------------------------------------------------
float Vector3f::SquareLength( ) const
{
	return x*x + y*y + z*z;
}


//-------------------------------------------------------------------------------------------------
Vector2f Vector3f::XY( )
{
	return Vector2f( x, y );
}