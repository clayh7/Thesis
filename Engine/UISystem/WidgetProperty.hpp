#pragma once

#include "Engine/UISystem/UICommon.hpp"


//-------------------------------------------------------------------------------------------------
template<typename T>
class WidgetProperty
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	T m_value;
	eWidgetPropertySource m_source;
	eWidgetState m_state;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	WidgetProperty( T const & value = T(), eWidgetPropertySource source = eWidgetPropertySource_CODE, eWidgetState state = eWidgetState_ALL )
		: m_value( value )
		, m_source( source )
		, m_state( state )
	{
		//Nothing
	}
};