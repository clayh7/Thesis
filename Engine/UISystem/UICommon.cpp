#include "Engine/UISystem/UICommon.hpp"


//-------------------------------------------------------------------------------------------------
eAlignment ParseAlignment( std::string const & alignment )
{
	if( strcmp( alignment.c_str( ), "left" ) == 0 )
	{
		return eAlignment_LEFT;
	}

	if( strcmp( alignment.c_str( ), "center" ) == 0 )
	{
		return eAlignment_CENTER;
	}

	if( strcmp( alignment.c_str( ), "right" ) == 0 )
	{
		return eAlignment_RIGHT;
	}

	return eAlignment_NONE;
}


//-------------------------------------------------------------------------------------------------
eAnchor ParseAnchor( std::string const & anchor )
{
	if( strcmp( anchor.c_str( ), "top-left" ) == 0 )
	{
		return eAnchor_TOP_LEFT;
	}

	if( strcmp( anchor.c_str( ), "top" ) == 0 )
	{
		return eAnchor_TOP;
	}

	if( strcmp( anchor.c_str( ), "top-right" ) == 0 )
	{
		return eAnchor_TOP_RIGHT;
	}

	if( strcmp( anchor.c_str( ), "left" ) == 0 )
	{
		return eAnchor_LEFT;
	}

	if( strcmp( anchor.c_str( ), "center" ) == 0 )
	{
		return eAnchor_CENTER;
	}

	if( strcmp( anchor.c_str( ), "right" ) == 0 )
	{
		return eAnchor_RIGHT;
	}

	if( strcmp( anchor.c_str( ), "bottom-left" ) == 0 )
	{
		return eAnchor_BOTTOM_LEFT;
	}

	if( strcmp( anchor.c_str( ), "bottom" ) == 0 )
	{
		return eAnchor_BOTTOM;
	}

	if( strcmp( anchor.c_str( ), "bottom-right" ) == 0 )
	{
		return eAnchor_BOTTOM_RIGHT;
	}

	return eAnchor_CENTER;
}


//-------------------------------------------------------------------------------------------------
eDock ParseDock( std::string const & dock )
{
	if( strcmp( dock.c_str( ), "right" ) == 0 )
	{
		return eDock_RIGHT;
	}

	if( strcmp( dock.c_str( ), "top" ) == 0 )
	{
		return eDock_TOP;
	}

	if( strcmp( dock.c_str( ), "left" ) == 0 )
	{
		return eDock_LEFT;
	}

	if( strcmp( dock.c_str( ), "bottom" ) == 0 )
	{
		return eDock_BOTTOM;
	}

	if( strcmp( dock.c_str( ), "fill" ) == 0 )
	{
		return eDock_FILL;
	}

	return eDock_NONE;
}


//-------------------------------------------------------------------------------------------------
eFillDirection ParseFillDirection( std::string const & fillDirection )
{
	if( strcmp( fillDirection.c_str( ), "left to right" ) == 0 )
	{
		return eFillDirection_LEFT_TO_RIGHT;
	}

	if( strcmp( fillDirection.c_str( ), "right" ) == 0 )
	{
		return eFillDirection_LEFT_TO_RIGHT;
	}

	if( strcmp( fillDirection.c_str( ), "right to left" ) == 0 )
	{
		return eFillDirection_RIGHT_TO_LEFT;
	}

	if( strcmp( fillDirection.c_str( ), "left" ) == 0 )
	{
		return eFillDirection_RIGHT_TO_LEFT;
	}

	if( strcmp( fillDirection.c_str( ), "bottom to top" ) == 0 )
	{
		return eFillDirection_BOTTOM_TO_TOP;
	}

	if( strcmp( fillDirection.c_str( ), "up" ) == 0 )
	{
		return eFillDirection_BOTTOM_TO_TOP;
	}

	if( strcmp( fillDirection.c_str( ), "top to bottom" ) == 0 )
	{
		return eFillDirection_TOP_TO_BOTTOM;
	}

	if( strcmp( fillDirection.c_str( ), "down" ) == 0 )
	{
		return eFillDirection_TOP_TO_BOTTOM;
	}

	return eFillDirection_LEFT_TO_RIGHT;
}


//-------------------------------------------------------------------------------------------------
eWidgetState ParseState( std::string const & state )
{
	if( strcmp( state.c_str( ), "disabled" ) == 0 )
	{
		return eWidgetState_DISABLED;
	}

	if( strcmp( state.c_str( ), "enabled" ) == 0 )
	{
		return eWidgetState_ENABLED;
	}

	if( strcmp( state.c_str( ), "highlighted" ) == 0 )
	{
		return eWidgetState_HIGHLIGHTED;
	}

	if( strcmp( state.c_str( ), "pressed" ) == 0 )
	{
		return eWidgetState_PRESSED;
	}

	if( strcmp( state.c_str( ), "all" ) == 0 )
	{
		return eWidgetState_ALL;
	}

	return eWidgetState_ALL;
}
