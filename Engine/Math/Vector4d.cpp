#include "Engine/Math/Vector4d.hpp"

#include "Engine/Math/Vector3d.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector4d Vector4d::ZERO( 0.f );
STATIC const Vector4d Vector4d::ONE( 1.f );


//-------------------------------------------------------------------------------------------------
Vector4d::Vector4d( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector4d::Vector4d( Vector4d const & setVector )
	: x( setVector.x )
	, y( setVector.y )
	, z( setVector.z )
	, w( setVector.w )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4d::Vector4d( double setValues )
	: x( setValues )
	, y( setValues )
	, z( setValues )
	, w( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4d::Vector4d( double setX, double setY, double setZ, double setW )
	: x( setX )
	, y( setY )
	, z( setZ )
	, w( setW )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4d::Vector4d( Vector3d const & setXYZ, double setW )
	: x( setXYZ.x )
	, y( setXYZ.y )
	, z( setXYZ.z )
	, w( setW )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4d::Vector4d( double setX, Vector3d const &setYZW )
	: x( setX )
	, y( setYZW.x )
	, z( setYZW.y )
	, w( setYZW.z )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4d const Vector4d::operator+( Vector4d const & add ) const
{
	return Vector4d( x + add.x, y + add.y, z + add.z, w + add.w );
}


//-------------------------------------------------------------------------------------------------
void Vector4d::operator+=( Vector4d const & add )
{
	x += add.x;
	y += add.y;
	z += add.z;
	w += add.w;
}


//-------------------------------------------------------------------------------------------------
Vector4d const Vector4d::operator-( ) const
{
	return Vector4d( -x, -y, -z, -w );
}


//-------------------------------------------------------------------------------------------------
Vector4d const Vector4d::operator-( Vector4d const & subtract ) const
{
	return Vector4d( x - subtract.x, y - subtract.y, z - subtract.z, w - subtract.w );
}


//-------------------------------------------------------------------------------------------------
void Vector4d::operator-=( Vector4d const & subtract )
{
	x -= subtract.x;
	y -= subtract.y;
	z -= subtract.z;
	w -= subtract.w;
}


//-------------------------------------------------------------------------------------------------
Vector4d const Vector4d::operator*( double rhs ) const
{
	return Vector4d( x * rhs, y * rhs, z * rhs, w * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector4d const operator*( double lhs, Vector4d const & rhs )
{
	return rhs * lhs;
}

//-------------------------------------------------------------------------------------------------
void Vector4d::operator*=( double rhs )
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	w *= rhs;
}


//-------------------------------------------------------------------------------------------------
bool Vector4d::operator==( Vector4d const & check ) const
{
	return ( x == check.x ) && ( y == check.y ) && ( z == check.z ) && ( w == check.w );
}


//-------------------------------------------------------------------------------------------------
void Vector4d::SetXYZW( double setX, double setY, double setZ, double setW )
{
	x = setX;
	y = setY;
	z = setZ;
	w = setW;
}


//-------------------------------------------------------------------------------------------------
void Vector4d::Normalize( )
{
	double length = Length( );
	if ( length == 0.f )
		return;
	double oneOverLength = 1.f / length;
	x *= oneOverLength;
	y *= oneOverLength;
	z *= oneOverLength;
	w *= oneOverLength;
}


//-------------------------------------------------------------------------------------------------
double Vector4d::Length( ) const
{
	return ( double ) sqrt( SquareLength( ) );
}


//-------------------------------------------------------------------------------------------------
double Vector4d::SquareLength( ) const
{
	return x*x + y*y + z*z + w*w;
}


//-------------------------------------------------------------------------------------------------
Vector3d Vector4d::XYZ( )
{
	return Vector3d( x, y, z );
}
