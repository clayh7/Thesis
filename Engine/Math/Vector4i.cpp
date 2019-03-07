#include "Engine/Math/Vector4i.hpp"

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector4i Vector4i::ZERO( 0 );
STATIC const Vector4i Vector4i::ONE( 1 );


//-------------------------------------------------------------------------------------------------
Vector4i::Vector4i( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector4i::Vector4i( int setValues )
	: x( setValues )
	, y( setValues )
	, z( setValues )
	, w( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4i::Vector4i( int setX, int setY, int setZ, int setW )
	: x( setX )
	, y( setY )
	, z( setZ )
	, w( setW )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector4i const Vector4i::operator+( Vector4i const & add ) const
{
	return Vector4i( x + add.x, y + add.y, z + add.z, w + add.w );
}


//-------------------------------------------------------------------------------------------------
Vector4i const Vector4i::operator-( ) const
{
	return Vector4i( -x, -y, -z, -w );
}


//-------------------------------------------------------------------------------------------------
Vector4i const Vector4i::operator-( Vector4i const & subtract ) const
{
	return Vector4i( x + subtract.x, y + subtract.y, z + subtract.z, w - subtract.w );
}


//-------------------------------------------------------------------------------------------------
Vector4i const Vector4i::operator*( int rhs ) const
{
	return Vector4i( x * rhs, y * rhs, z * rhs, w * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector4i const operator*( int lhs, Vector4i const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
bool Vector4i::operator==( Vector4i const & check ) const
{
	return ( x == check.x ) && ( y == check.y ) && ( z == check.z ) && ( w == check.w );
}


//-------------------------------------------------------------------------------------------------
bool Vector4i::operator!=( Vector4i const & check ) const
{
	return ( x != check.x ) || ( y != check.y ) || ( z != check.z ) || ( w != check.w );
}
