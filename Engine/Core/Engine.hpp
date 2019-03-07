#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include "Engine/RenderSystem/Color.hpp"


//-------------------------------------------------------------------------------------------------
class Camera3D;
class Engine;
class Vector2i;
class Vector3f;
class Vector2f;
extern Engine * g_EngineSystem;


//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam );
typedef void ( MessagePumpCallback )( void );


//-------------------------------------------------------------------------------------------------
class Engine
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * APP_NAME;
	static int const START_WINDOW_OFFSET = 50;
	static int const START_WINDOW_WIDTH = 1600;
	static int const START_WINDOW_HEIGHT = 900;
	static float const PERCENT_OF_SCREEN;
	static float const ASPECT_RATIO;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static Vector2i GetWindowDimensions( );
	static Vector2i GetDesktopDimensions( );
	static float GetWindowAspectRatio( );
	static Vector3f ScreenToClipSpace( Vector2f const &screenPosition );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	HWND m_windowHandle = nullptr;
	HDC g_displayDeviceContext = nullptr;
	HGLRC g_openGLRenderingContext = nullptr;
	HINSTANCE m_applicationInstanceHandle;
	double m_timeLastFrameBegan;
	double m_targetFPS;
	bool m_isFullscreen;
	Camera3D * m_UICamera; //#TODO: Make this something that's passed into the UI system, and all MeshRenderer calls use this camera proj/view stuff
	int m_currentDrawCalls; //#TODO: Put this in RenderSystem

	int m_offsetXFromWindowsDesktop = START_WINDOW_OFFSET;
	int m_offsetYFromWindowsDesktop = START_WINDOW_OFFSET;
	int m_windowPhysicalWidth = START_WINDOW_WIDTH;
	int m_windowPhysicalHeight = START_WINDOW_HEIGHT;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Engine( HINSTANCE applicationInstanceHandle );
	~Engine( );

	void Update( );
	void LateUpdate( );
	void UpdateTime( );
	void Render( ) const;
	void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle );
	void CalculateLargestWindow( );

	void SetTargetFPS( double fps );
	void SetWindowHandle( HWND & handle );
	void SetFullscreen( bool isFullscreen );

	void IncrementDrawCalls( );
	int GetCurrentDrawCalls( ) const;
	HWND GetWindowHandle( ) const;
	double GetTargetFPS( ) const;

	void ConsolePrint( std::string const & consoleLog, Color const & color = Color::WHITE );
	void ConsolePrintf( const char* format, ... );
	void ConsolePrintf( Color const & color, const char* format, ... );
};