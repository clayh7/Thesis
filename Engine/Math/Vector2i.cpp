#include "Engine/Math/Vector2i.hpp"

#include <vector>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector2i Vector2i::ZERO( 0 );
STATIC const Vector2i Vector2i::ONE( 1 );


//-------------------------------------------------------------------------------------------------
Vector2i::Vector2i( )
	: Vector2i( 0 )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2i::Vector2i( int setValues )
	: x( setValues )
	, y( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2i::Vector2i( int setX, int setY )
	: x( setX )
	, y( setY )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
//#TODO: Support parsing "," commas also
Vector2i::Vector2i( std::string const & valueRange )
{
	//If the string has commas, use comma splitter
	char splitToken = ',';
	size_t foundComma = valueRange.find( ',' );
	//Otherwise use ~ as the comma splitter
	if( foundComma == std::string::npos )
	{
		splitToken = '~';
	}
	std::vector<std::string> values = SplitString( valueRange, splitToken );
	if ( values.size( ) == 0 )
	{
		x = 0;
		y = 0;
	}
	else if ( values.size( ) == 1 )
	{
		SetTypeFromString( x, values[0] );
		SetTypeFromString( y, values[0] );
	}
	else if( values.size( ) == 2 )
	{
		SetTypeFromString( x, values[0] );
		SetTypeFromString( y, values[1] );
	}
	else
	{
		ERROR_AND_DIE( "Parsing Vector2i from std::string format not supported" );
	}
}


//-------------------------------------------------------------------------------------------------
Vector2i const Vector2i::operator+( Vector2i const &add ) const
{
	return Vector2i( x + add.x, y + add.y );
}


//-------------------------------------------------------------------------------------------------
void Vector2i::operator+=( Vector2i const &add )
{
	x = x + add.x;
	y = y + add.y;
}


//-------------------------------------------------------------------------------------------------
Vector2i const Vector2i::operator-( ) const
{
	return Vector2i( -x, -y );
}


//-------------------------------------------------------------------------------------------------
Vector2i const Vector2i::operator-( Vector2i const & subtract ) const
{
	return Vector2i( x - subtract.x, y - subtract.y );
}


//-------------------------------------------------------------------------------------------------
Vector2i const Vector2i::operator*( int rhs ) const
{
	return Vector2i( x * rhs, y * rhs );
}


//-------------------------------------------------------------------------------------------------
Vector2i const operator*( int lhs, Vector2i const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
bool Vector2i::operator==( Vector2i const & check ) const
{
	return ( x == check.x ) && ( y == check.y );
}


//-------------------------------------------------------------------------------------------------
bool Vector2i::operator<( Vector2i const & check ) const
{
	if ( x < check.x )
	{
		return true;
	}
	else if ( check.x == x && y < check.y )
	{
		return true;
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
int Vector2i::SquareLength( ) const
{
	return x*x + y*y;
}


//-------------------------------------------------------------------------------------------------
std::string Vector2i::ToString( ) const
{
	return Stringf( "%d~%d", x, y );
}
