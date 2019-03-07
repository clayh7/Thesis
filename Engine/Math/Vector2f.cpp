 #include "Engine/Math/Vector2f.hpp"

#include <vector>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector2i.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Vector2f Vector2f::ZERO( 0.f );
STATIC const Vector2f Vector2f::ONE( 1.f );


//-------------------------------------------------------------------------------------------------
Vector2f::Vector2f( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector2f::Vector2f( float setValues )
	: x( setValues )
	, y( setValues )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2f::Vector2f( Vector2f const &setVector )
	: x( setVector.x )
	, y( setVector.y )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2f::Vector2f( float setX, float setY )
	: x( setX )
	, y( setY )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2f::Vector2f( Vector2i const & setVector )
	: x( (float)setVector.x )
	, y( (float)setVector.y )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2f::Vector2f( std::string valuePair )
{
	if( valuePair.size() == 0 )
	{
		x = 0;
		y = 0;
		return;
	}

	size_t rangePosition = valuePair.find( '~' );
	size_t commaPosition = valuePair.find( ',' );
	if( rangePosition < valuePair.size( ) )
	{
		std::vector<std::string> values = SplitString( valuePair, '~' );
		if( values.size( ) == 1 )
		{
			SetTypeFromString( x, values[0] );
			SetTypeFromString( y, values[0] );
		}
		else if( values.size( ) == 2 )
		{
			SetTypeFromString( x, values[0] );
			SetTypeFromString( y, values[1] );
		}
	}
	else if( commaPosition < valuePair.size() )
	{
		std::vector<std::string> values = SplitString( valuePair, ',' );
		if( values.size( ) == 1 )
		{
			SetTypeFromString( x, values[0] );
			SetTypeFromString( y, values[0] );
		}
		else if( values.size( ) == 2 )
		{
			SetTypeFromString( x, values[0] );
			SetTypeFromString( y, values[1] );
		}
	}
	else
	{
		ERROR_AND_DIE( "Parsing Vector2f from std::string format not supported" );
	}
}


//-------------------------------------------------------------------------------------------------
Vector2f const Vector2f::operator+( Vector2f const &add ) const
{
	return Vector2f( x + add.x, y + add.y );
}


//-------------------------------------------------------------------------------------------------
void Vector2f::operator+=( Vector2f const &add )
{
	x += add.x;
	y += add.y;
}


//-------------------------------------------------------------------------------------------------
Vector2f const Vector2f::operator-( Vector2f const &subtract ) const
{
	return Vector2f( x - subtract.x, y - subtract.y );
}


//-------------------------------------------------------------------------------------------------
void Vector2f::operator-=( Vector2f const &subtract )
{
	x -= subtract.x;
	y -= subtract.y;
}


//-------------------------------------------------------------------------------------------------
Vector2f const Vector2f::operator*( Vector2f const & rhs ) const
{
	return Vector2f( x * rhs.x, y * rhs.y );
}


//-------------------------------------------------------------------------------------------------
Vector2f const operator*( float lhs, Vector2f const & rhs )
{
	return rhs * lhs;
}


//-------------------------------------------------------------------------------------------------
void Vector2f::operator*=( float rhs )
{
	x *= rhs;
	y *= rhs;
}


//-------------------------------------------------------------------------------------------------
Vector2f const Vector2f::operator/( Vector2f const & rhs )
{
	return Vector2f( x / rhs.x, y / rhs.y );
}


//-------------------------------------------------------------------------------------------------
Vector2f const Vector2f::operator-( ) const
{
	return Vector2f( -x, -y );
}


//-------------------------------------------------------------------------------------------------
bool Vector2f::operator==( Vector2f const & check ) const
{
	return ( x == check.x ) && ( y == check.y );
}


//-------------------------------------------------------------------------------------------------
bool Vector2f::operator!=( Vector2f const & check ) const
{
	return ( x != check.x ) || ( y != check.y );
}


//-------------------------------------------------------------------------------------------------
bool Vector2f::operator<=( Vector2f const & rhs ) const
{
	return ( x <= rhs.x ) && ( y <= rhs.y );
}


//-------------------------------------------------------------------------------------------------
void Vector2f::SetXY( float setX, float setY )
{
	x = setX;
	y = setY;
}


//-------------------------------------------------------------------------------------------------
Vector2f Vector2f::Normal( )
{
	Vector2f normal = Vector2f( x, y );
	normal.Normalize( );
	return normal;
}


//-------------------------------------------------------------------------------------------------
void Vector2f::Normalize( )
{
	float length = Length( );
	if( length <= 0.f )
	{
		return;
	}
	float oneOverLength = 1.f / length;
	x *= oneOverLength;
	y *= oneOverLength;
}


//-------------------------------------------------------------------------------------------------
float Vector2f::Length( ) const
{
	return sqrt( (float) SquareLength( ) );
}


//-------------------------------------------------------------------------------------------------
float Vector2f::SquareLength( ) const
{
	return x*x + y*y;
}


//-------------------------------------------------------------------------------------------------
void Vector2f::Rotate( float degrees )
{
	float tx = x;
	float ty = y;
	x = CosDegrees( degrees ) * tx - SinDegrees( degrees ) * ty;
	y = SinDegrees( degrees ) * tx + CosDegrees( degrees ) * ty;
}