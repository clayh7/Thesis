#pragma once

#include <string>


//-------------------------------------------------------------------------------------------------
enum eAlignment
{
	eAlignment_LEFT,
	eAlignment_CENTER,
	eAlignment_RIGHT,
	eAlignment_NONE,
};


//-------------------------------------------------------------------------------------------------
eAlignment ParseAlignment( std::string const & alignment );


//-------------------------------------------------------------------------------------------------
enum eAnchor
{
	eAnchor_TOP_LEFT,
	eAnchor_TOP,
	eAnchor_TOP_RIGHT,
	eAnchor_LEFT,
	eAnchor_CENTER,
	eAnchor_RIGHT,
	eAnchor_BOTTOM_LEFT,
	eAnchor_BOTTOM,
	eAnchor_BOTTOM_RIGHT,
};


//-------------------------------------------------------------------------------------------------
eAnchor ParseAnchor( std::string const & anchor );


//-------------------------------------------------------------------------------------------------
enum eDock
{
	eDock_NONE,
	eDock_RIGHT,
	eDock_TOP,
	eDock_LEFT,
	eDock_BOTTOM,
	eDock_FILL,
};


//-------------------------------------------------------------------------------------------------
eDock ParseDock( std::string const & dock );


//-------------------------------------------------------------------------------------------------
enum eFillDirection
{
	eFillDirection_LEFT_TO_RIGHT,
	eFillDirection_RIGHT_TO_LEFT,
	eFillDirection_BOTTOM_TO_TOP,
	eFillDirection_TOP_TO_BOTTOM,
};


//-------------------------------------------------------------------------------------------------
eFillDirection ParseFillDirection( std::string const & fillDirection );


//-------------------------------------------------------------------------------------------------
enum eWidgetState
{
	eWidgetState_DISABLED,
	eWidgetState_ENABLED,
	eWidgetState_HIGHLIGHTED,
	eWidgetState_PRESSED,
	eWidgetState_ALL,
	eWidgetState_COUNT,
};


//-------------------------------------------------------------------------------------------------
eWidgetState ParseState( std::string const & state );


//-------------------------------------------------------------------------------------------------
enum eWidgetPropertySource
{
	eWidgetPropertySource_CODE,
	eWidgetPropertySource_SKIN,
	eWidgetPropertySource_WIDGET_SPECIFIC,
};