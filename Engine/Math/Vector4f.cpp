#include "Engine/Math/Vector4f.hpp"

#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector4f Vector4f::ZERO( 0.f );
STATIC const Vector4f Vector4f::ONE( 1.f );


//-------------------------------------------------------------------------------------------------
Vector4f::Vector4f( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector4f::Vector4f( Vector4f const & setVector )
	: x( setVector.x )
	, y( setVector.y )
	, z( setVector.z )
	, w( setVector.w )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4f::Vector4f( float setValues )
	: x( setValues )
	, y( setValues )
	, z( setValues )
	, w( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4f::Vector4f( float setX, float setY, float setZ, float setW )
	: x( setX )
	, y( setY )
	, z( setZ )
	, w( setW )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4f::Vector4f( Vector3f const & setXYZ, float setW )
	: x( setXYZ.x )
	, y( setXYZ.y )
	, z( setXYZ.z )
	, w( setW )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4f::Vector4f( float setX, Vector3f const & setYZW )
	: x( setX )
	, y( setYZW.x )
	, z( setYZW.y )
	, w( setYZW.z )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4f::Vector4f( Vector2f const & setXY, float setZ, float setW )
	: x( setXY.x )
	, y( setXY.y )
	, z( setZ )
	, w( setW )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4f const Vector4f::operator+( Vector4f const & add ) const
{
	return Vector4f( x + add.x, y + add.y, z + add.z, w + add.w );
}


//-------------------------------------------------------------------------------------------------
void Vector4f::operator+=( Vector4f const & add )
{
	x += add.x;
	y += add.y;
	z += add.z;
	w += add.w;
}


//-------------------------------------------------------------------------------------------------
Vector4f const Vector4f::operator-( ) const
{
	return Vector4f( -x, -y, -z, -w );
}


//-------------------------------------------------------------------------------------------------
Vector4f const Vector4f::operator-( Vector4f const & subtract ) const
{
	return Vector4f( x - subtract.x, y - subtract.y, z - subtract.z, w - subtract.w );
}


//-------------------------------------------------------------------------------------------------
void Vector4f::operator-=( Vector4f const & subtract )
{
	x -= subtract.x;
	y -= subtract.y;
	z -= subtract.z;
	w -= subtract.w;
}


//-------------------------------------------------------------------------------------------------
Vector4f const Vector4f::operator*( float rhs ) const
{
	return Vector4f( x * rhs, y * rhs, z * rhs, w * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector4f const operator*( float lhs, Vector4f const & rhs )
{
	return rhs * lhs;
}

//-------------------------------------------------------------------------------------------------
void Vector4f::operator*=( float rhs )
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	w *= rhs;
}


//-------------------------------------------------------------------------------------------------
bool Vector4f::operator==( Vector4f const & check ) const
{
	return ( x == check.x ) && ( y == check.y ) && ( z == check.z ) && ( w == check.w );
}


//-------------------------------------------------------------------------------------------------
void Vector4f::SetXYZW( float setX, float setY, float setZ, float setW )
{
	x = setX;
	y = setY;
	z = setZ;
	w = setW;
}


//-------------------------------------------------------------------------------------------------
void Vector4f::Normalize( )
{
	float length = Length( );
	if ( length == 0.f )
		return;
	float oneOverLength = 1.f / length;
	x *= oneOverLength;
	y *= oneOverLength;
	z *= oneOverLength;
	w *= oneOverLength;
}


//-------------------------------------------------------------------------------------------------
float Vector4f::Length( ) const
{
	return ( float ) sqrt( SquareLength( ) );
}


//-------------------------------------------------------------------------------------------------
float Vector4f::SquareLength( ) const
{
	return x*x + y*y + z*z + w*w;
}


//-------------------------------------------------------------------------------------------------
Vector3f Vector4f::XYZ( ) const
{
	return Vector3f( x, y, z );
}


//-------------------------------------------------------------------------------------------------
Vector2f Vector4f::XY( ) const
{
	return Vector2f( x, y );
}