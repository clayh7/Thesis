#pragma once

#include <string>


//-------------------------------------------------------------------------------------------------
class Vector2i
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const Vector2i ZERO;
	static const Vector2i ONE;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	union
	{
		int x;
		int min;
	};
	union
	{
		int y;
		int max;
	};

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vector2i( );
	Vector2i( int setValues );
	Vector2i( int setX, int setY );
	Vector2i( std::string const & valueRange );

	Vector2i const operator+( Vector2i const & add ) const;
	void operator+=( Vector2i const & add );
	Vector2i const operator-( ) const;
	Vector2i const operator-( Vector2i const & subtract ) const;
	Vector2i const operator*( int rhs ) const;
	friend Vector2i const operator*( int lhs, Vector2i const & rhs );
	bool Vector2i::operator==( Vector2i const & check ) const;
	bool Vector2i::operator<( Vector2i const & check ) const;

	int Vector2i::SquareLength( ) const;
	std::string ToString( ) const;
};