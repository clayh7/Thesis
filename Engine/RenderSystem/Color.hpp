#pragma once

#include <string>
#include "Engine/Math/Vector4f.hpp"


//-------------------------------------------------------------------------------------------------
class Color
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const Color WHITE;
	static const Color LIGHT_GREY;
	static const Color GREY;
	static const Color DARK_GREY;
	static const Color BLACK;
	static const Color LIGHT_RED;
	static const Color RED;
	static const Color DARK_RED;
	static const Color ORANGE;
	static const Color YELLOW;
	static const Color LIGHT_GREEN;
	static const Color GREEN;
	static const Color DARK_GREEN;
	static const Color TURQUOISE;
	static const Color CYAN;
	static const Color LIGHT_BLUE;
	static const Color BLUE;
	static const Color DARK_BLUE;
	static const Color PURPLE;
	static const Color VIOLET;
	static const Color MAGENTA;
	static const Color PINK;
	static const Color BROWN;
	static const Color CLAY_GREEN;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static Color Lerp( Color const & startColor, Color const & endColor, float zeroToOne );
	static Color const RandomColor( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Color( );
	Color( unsigned char setR, unsigned char setG, unsigned char setB, unsigned char setA );
	Color( std::string colorString );

	bool Color::operator==( Color const & check ) const;
	bool Color::operator!=( Color const & check ) const;
	void Multiply( float darkenPercent );

	float GetRFloat( ) const;
	float GetGFloat( ) const;
	float GetBFloat( ) const;
	float GetAFloat( ) const;
	Vector4f GetVector4f( ) const;
};