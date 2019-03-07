#include "Engine/RenderSystem/Color.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const Color Color::WHITE( 255, 255, 255, 255 );
STATIC const Color Color::LIGHT_GREY( 200, 200, 200, 255 );
STATIC const Color Color::GREY( 127, 127, 127, 255 );
STATIC const Color Color::DARK_GREY( 50, 50, 50, 255 );
STATIC const Color Color::BLACK( 0, 0, 0, 255 );
STATIC const Color Color::LIGHT_RED( 255, 125, 125, 255 );
STATIC const Color Color::RED( 255, 0, 0, 255 );
STATIC const Color Color::DARK_RED( 127, 0, 0, 255 );
STATIC const Color Color::ORANGE( 255, 125, 0, 255 );
STATIC const Color Color::YELLOW( 255, 255, 0, 255);
STATIC const Color Color::LIGHT_GREEN( 125, 255, 0, 255 );
STATIC const Color Color::GREEN( 0, 255, 0, 255 );
STATIC const Color Color::DARK_GREEN( 0, 127, 0, 255 );
STATIC const Color Color::TURQUOISE( 0, 255, 125, 255 );
STATIC const Color Color::CYAN( 0, 255, 255, 255 );
STATIC const Color Color::LIGHT_BLUE( 0, 125, 255, 255 );
STATIC const Color Color::BLUE( 0, 0, 255, 255 );
STATIC const Color Color::DARK_BLUE( 0, 0, 125, 255 );
STATIC const Color Color::PURPLE( 125, 0, 255, 255 );
STATIC const Color Color::VIOLET( 125, 0, 255, 255 );
STATIC const Color Color::MAGENTA( 255, 0, 255, 255 );
STATIC const Color Color::PINK( 255, 0, 255, 255 );
STATIC const Color Color::BROWN( 150, 75, 0, 255 );
STATIC const Color Color::CLAY_GREEN( 10, 63, 53, 255 );


//-------------------------------------------------------------------------------------------------
STATIC Color Color::Lerp( Color const & startColor, Color const & endColor, float zeroToOne )
{
	float percent = Clamp( zeroToOne, 0.f, 1.f );
	float newRfloat = ( startColor.GetRFloat( ) * ( 1 - percent ) ) + ( endColor.GetRFloat( ) * percent );
	float newGfloat = ( startColor.GetGFloat( ) * ( 1 - percent ) ) + ( endColor.GetGFloat( ) * percent );
	float newBfloat = ( startColor.GetBFloat( ) * ( 1 - percent ) ) + ( endColor.GetBFloat( ) * percent );
	float newAfloat = ( startColor.GetAFloat( ) * ( 1 - percent ) ) + ( endColor.GetAFloat( ) * percent );
	unsigned char newR = (unsigned char) Clamp( 255.f*newRfloat, 0.f, 255.f );
	unsigned char newG = (unsigned char) Clamp( 255.f*newGfloat, 0.f, 255.f );
	unsigned char newB = (unsigned char) Clamp( 255.f*newBfloat, 0.f, 255.f );
	unsigned char newA = (unsigned char) Clamp( 255.f*newAfloat, 0.f, 255.f );
	return Color( newR, newG, newB, newA );
}


//-------------------------------------------------------------------------------------------------
STATIC Color const Color::RandomColor( )
{
	unsigned char r = (unsigned char) RandomInt( 0, 255 );
	unsigned char g = (unsigned char) RandomInt( 0, 255 );
	unsigned char b = (unsigned char) RandomInt( 0, 255 );
	return Color( r, g, b, 255 );
}


//-------------------------------------------------------------------------------------------------
Color::Color( )
	: Color( BLACK )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Color::Color( unsigned char setR, unsigned char setG, unsigned char setB, unsigned char setA )
	: r( setR )
	, g( setG )
	, b( setB )
	, a( setA )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Color::Color( std::string colorString )
{
	std::vector<std::string> colorValues = SplitString( colorString, ',' );

	if( colorValues.size( ) == 1 )
	{
		SetTypeFromString( r, colorValues[0] );
		SetTypeFromString( g, colorValues[0] );
		SetTypeFromString( b, colorValues[0] );
		a = 255;
	}
	else if( colorValues.size( ) == 2 )
	{
		SetTypeFromString( r, colorValues[0] );
		SetTypeFromString( g, colorValues[0] );
		SetTypeFromString( b, colorValues[0] );
		SetTypeFromString( a, colorValues[1] );
	}
	else if( colorValues.size( ) == 3 )
	{
		SetTypeFromString( r, colorValues[0] );
		SetTypeFromString( g, colorValues[1] );
		SetTypeFromString( b, colorValues[2] );
		a = 255;
	}
	else if( colorValues.size( ) == 4 )
	{
		SetTypeFromString( r, colorValues[0] );
		SetTypeFromString( g, colorValues[1] );
		SetTypeFromString( b, colorValues[2] );
		SetTypeFromString( a, colorValues[3] );
	}
}


//-------------------------------------------------------------------------------------------------
bool Color::operator==( Color const & check ) const
{
	return ( r == check.r ) && ( g == check.g ) && ( b == check.b ) && ( a == check.a );
}


//-------------------------------------------------------------------------------------------------
bool Color::operator!=( Color const & check ) const
{
	return  ( r != check.r ) || ( g != check.g ) || ( b != check.b ) || ( a != check.a );
}


//-------------------------------------------------------------------------------------------------
void Color::Multiply( float darkenPercent )
{
	r = (unsigned char) Clamp( ( r * darkenPercent ), 0.f, 255.f );
	g = (unsigned char) Clamp( ( g * darkenPercent ), 0.f, 255.f );
	b = (unsigned char) Clamp( ( b * darkenPercent ), 0.f, 255.f );
}


//-------------------------------------------------------------------------------------------------
float Color::GetRFloat( ) const
{
	return ( float )r / 255.f;
}


//-------------------------------------------------------------------------------------------------
float Color::GetGFloat( ) const
{
	return ( float )g / 255.f;
}


//-------------------------------------------------------------------------------------------------
float Color::GetBFloat( ) const
{
	return ( float )b / 255.f;
}


//-------------------------------------------------------------------------------------------------
float Color::GetAFloat( ) const
{
	return ( float )a / 255.f;
}


//-------------------------------------------------------------------------------------------------
Vector4f Color::GetVector4f( ) const
{
	return Vector4f( GetRFloat( ), GetGFloat( ), GetBFloat( ), GetAFloat( ) );
}