#pragma once

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Math/Vector2f.hpp"


//-------------------------------------------------------------------------------------------------
template<typename Type>
class Range
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Range<Type> const ZERO;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	Type minimum;
	Type maximum;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	//-------------------------------------------------------------------------------------------------
	Range( Type v )
		: minimum( v )
		, maximum( v )
	{
		//Nothing
	}


	//-------------------------------------------------------------------------------------------------
	Range( Type minValue, Type maxValue )
		: minimum( minValue )
		, maximum( maxValue )
	{
		ASSERT_RECOVERABLE( minimum <= maximum, "Min must be smaller than max" );
	}


	//-------------------------------------------------------------------------------------------------
	Type GetRandom( ) const
	{
		if( minimum == maximum )
		{
			return minimum;
		}
		else
		{
			return Get( RandomFloatZeroToOne( ) );
		}
	}


	//-------------------------------------------------------------------------------------------------
	Type Get( float valueZeroToOne ) const
	{
		valueZeroToOne = Clamp( valueZeroToOne, 0.f, 1.f );
		Type range = maximum - minimum;
		return minimum + (Type) ( range * valueZeroToOne );
	}


	//-------------------------------------------------------------------------------------------------
	bool const operator==( Range<Type> const & check ) const
	{
		return ( minimum == check.minimum ) && ( maximum == check.maximum );
	}
};


//-------------------------------------------------------------------------------------------------
template<typename Type>
Range<Type> const Range<Type>::ZERO( (Type) 0 );