#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Engine.hpp"
#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------------------------------------
bool g_isQuitting = false;
bool g_limitFPS = true;
int DRAW_CALLS = 0;
char const * const ENGINE_UPDATE_EVENT = "NetworkUpdate";
char const * const ENGINE_RENDER_EVENT = "EngineRender";


//-------------------------------------------------------------------------------------------------
// Global Functions
//-------------------------------------------------------------------------------------------------
Vector2i GetWindowDimensions( )
{
	return Engine::GetWindowDimensions( );
}
