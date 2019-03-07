#include "Engine/Math/Vector3i.hpp"

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector3i Vector3i::ZERO( 0 );
STATIC const Vector3i Vector3i::ONE( 1 );


//-------------------------------------------------------------------------------------------------
Vector3i::Vector3i( int setValues )
	: x( setValues )
	, y( setValues )
	, z( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3i::Vector3i( int setX, int setY, int setZ )
	: x( setX )
	, y( setY )
	, z( setZ )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector3i const Vector3i::operator+( Vector3i const & add ) const
{
	return Vector3i( x + add.x, y + add.y, z + add.z );
}


//-------------------------------------------------------------------------------------------------
Vector3i const Vector3i::operator-( ) const
{
	return Vector3i( -x, -y, -z );
}


//-------------------------------------------------------------------------------------------------
Vector3i const Vector3i::operator-( Vector3i const & subtract ) const
{
	return Vector3i( x + subtract.x, y + subtract.y, z + subtract.z );
}


//-------------------------------------------------------------------------------------------------
Vector3i const Vector3i::operator*( int rhs ) const
{
	return Vector3i( x * rhs, y * rhs, z * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector3i const operator*( int lhs, Vector3i const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
bool Vector3i::operator==( Vector3i const & check ) const
{
	return ( x == check.x ) && ( y == check.y ) && ( z == check.z );
}


//-------------------------------------------------------------------------------------------------
bool Vector3i::operator!=( Vector3i const & check ) const
{
	return ( x != check.x ) || ( y != check.y ) || ( z != check.z );
}
