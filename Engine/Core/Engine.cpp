//-------------------------------------------------------------------------------------------------
//"All the best weapons have names"
//This Engine is named: Bread
//Author: Clay Howell
// - Use with game jam
#include "Engine/Core/Engine.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>

#include "Engine/AudioSystem/Audio.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/Debugger.hpp"
#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/Net/NetworkSystem.hpp"
#include "Engine/Net/RCS/RemoteCommandServer.hpp"
#include "Engine/RenderSystem/Camera3D.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/Threads/JobSystem.hpp"
#include "Engine/UISystem/UISystem.hpp"


//-------------------------------------------------------------------------------------------------
extern Engine * g_EngineSystem = nullptr;


//-------------------------------------------------------------------------------------------------
//Name on top game window
STATIC char const * Engine::APP_NAME = "Bread v0.8";
STATIC float const Engine::PERCENT_OF_SCREEN = 0.8f;
STATIC float const Engine::ASPECT_RATIO = 16.f / 9.f;


//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	//Only run if these both exist
	if( !g_ConsoleSystem || !g_InputSystem )
	{
		return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
	}

	unsigned char asKey = (unsigned char) wParam;
	
	switch( wmMessageCode )
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		g_isQuitting = true;
		return 0;

	case WM_KEYDOWN:
		if( g_ConsoleSystem->IsOpen( ) )
		{
			g_ConsoleSystem->PressKey( asKey );
		}
		else
		{
			g_InputSystem->SetKeyStatus( asKey, true );
		}
		break;

	case WM_KEYUP:
		if( g_ConsoleSystem->IsOpen( ) )
		{
			g_ConsoleSystem->ReleaseKey( asKey );
		}
		else
		{
			g_InputSystem->SetKeyStatus( asKey, false );
		}
		break;

	case WM_KILLFOCUS:
		g_InputSystem->SetFocus( false );
		break;

	//This gets called before input is created
	case WM_SETFOCUS:
		if( g_InputSystem ) //Make sure it exists first
		{
			g_InputSystem->SetFocus( true );
		}
		break;

	case WM_LBUTTONDOWN:
		if( !g_ConsoleSystem->IsOpen( ) )
		{
			g_InputSystem->SetMouseStatus( eMouseButton_LEFT, true );
		}
		break;

	case WM_LBUTTONUP:
		if( !g_ConsoleSystem->IsOpen( ) )
		{
			g_InputSystem->SetMouseStatus( eMouseButton_LEFT, false );
		}
		break;

	case WM_RBUTTONDOWN:
		if( !g_ConsoleSystem->IsOpen( ) )
		{
			g_InputSystem->SetMouseStatus( eMouseButton_RIGHT, true );
		}
		break;

	case WM_RBUTTONUP:
		if( !g_ConsoleSystem->IsOpen( ) )
		{
			g_InputSystem->SetMouseStatus( eMouseButton_RIGHT, false );
		}
		break;

	case WM_MOUSEWHEEL:
		if( g_ConsoleSystem->IsOpen( ) )
		{
			g_ConsoleSystem->MoveMouseWheel( (int) GET_WHEEL_DELTA_WPARAM( wParam ) );
		}
		else
		{
			g_InputSystem->SetWheelStatus( (int) GET_WHEEL_DELTA_WPARAM( wParam ) );
		}
		break;

	case WM_CHAR:
		g_InputSystem->AddTypedCharacter( asKey );
		break;
	}

	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//-------------------------------------------------------------------------------------------------
Engine::Engine( HINSTANCE applicationInstanceHandle )
	: m_applicationInstanceHandle( applicationInstanceHandle )
	, m_timeLastFrameBegan( 0.0 )
	, m_targetFPS( 60.0 )
	, m_isFullscreen( false )
	, m_UICamera( nullptr )
	, m_currentDrawCalls( 0 )
{
	//Randomize Seed
	srand( (unsigned int) time( NULL ) );

	//Give us correct pixels on screen
	SetProcessDPIAware( );

	//Create Application Window
	CreateOpenGLWindow( m_applicationInstanceHandle );

	//Start Engine clock
	m_timeLastFrameBegan = Time::GetCurrentTimeSeconds( );

	//Create All Engine Systems
	g_MemoryAnalyticsSystem = new MemoryAnalytics( );
	EventSystem::Startup( );
	g_AudioSystem = new BAudio( );
	g_InputSystem = new Input( );
	g_RenderSystem = new Renderer( );
	m_UICamera = new Camera3D( false );
	g_RenderSystem->SetActiveCamera( m_UICamera );
	Console::Startup( );
	g_SpriteRenderSystem = new SpriteGameRenderer( );
	g_UISystem = new UISystem( );
	g_UISystem->LoadUIFromXML( );
	g_DebugSystem = new Debugger( );
	
	//Add a sleep(10) to the job threads
	//g_JobSystem = new JobSystem( );
	//g_JobSystem->Startup( -2 );
	NetworkSystem::Startup( );
	RemoteCommandServer::Startup( );
}


//-------------------------------------------------------------------------------------------------
Engine::~Engine( )
{
	RemoteCommandServer::Shutdown( );
	NetworkSystem::Shutdown( );

	//delete g_JobSystem;
	//g_JobSystem = nullptr;

	delete m_UICamera;
	m_UICamera = nullptr;

	delete g_ConsoleSystem;
	g_ConsoleSystem = nullptr;
	
	delete g_DebugSystem;
	g_DebugSystem = nullptr;

	delete g_UISystem;
	g_UISystem = nullptr;

	delete g_SpriteRenderSystem;
	g_SpriteRenderSystem = nullptr;

	delete g_RenderSystem;
	g_RenderSystem = nullptr;
	
	delete g_InputSystem;
	g_InputSystem = nullptr;

	delete g_AudioSystem;
	g_AudioSystem = nullptr;

	EventSystem::Shutdown( );

	Clock::DestroyClocks( );

	delete g_MemoryAnalyticsSystem;
	g_MemoryAnalyticsSystem = nullptr;
}



//-------------------------------------------------------------------------------------------------
void RunMessagePump( )
{
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}

//-------------------------------------------------------------------------------------------------
void Engine::Update( )
{
	//Update Total time and Delta time
	UpdateTime( );
	
	g_ProfilerSystem->StartSample( "UPDATE ENGINE" );
	
	//#TODO: Put these two and the Message Pump in InputSystem
	g_InputSystem->AdvanceFrameNumber( );
	RunMessagePump( );

	m_UICamera->Update( );
	g_InputSystem->Update( );
	g_MemoryAnalyticsSystem->Update( );
	g_AudioSystem->Update( );
	g_DebugSystem->Update( );
	g_ConsoleSystem->Update( );

	EventSystem::TriggerEvent( ENGINE_UPDATE_EVENT );

	//Reset Variables
	m_currentDrawCalls = 0;
	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void Engine::LateUpdate( )
{
	g_UISystem->Update( );
}


//-------------------------------------------------------------------------------------------------
void Engine::UpdateTime( )
{
	//#TODO: Find a nicer way to limit FPS
	g_ProfilerSystem->StartSample( "WAIT FOR FRAME" );
	double targetFPS = m_targetFPS;
	double timeThisFrameBegan = Time::GetCurrentTimeSeconds( );
	double elapsedTime = ( timeThisFrameBegan - m_timeLastFrameBegan );
	double totalFrameTime = 1.0 / targetFPS;

	if( g_limitFPS && elapsedTime < totalFrameTime )
	{
		double waitTime = (totalFrameTime - elapsedTime) * 1000.0 - 0.5;
		std::this_thread::sleep_for( std::chrono::duration<double,std::milli>( waitTime ) );
	}
	g_ProfilerSystem->StopSample( );

	//Track time
	timeThisFrameBegan = Time::GetCurrentTimeSeconds( );
	Time::DELTA_SECONDS = static_cast<float>( timeThisFrameBegan - m_timeLastFrameBegan );
	Time::TOTAL_SECONDS += Time::DELTA_SECONDS;
	m_timeLastFrameBegan = timeThisFrameBegan;

	//Update clock system
	for( Clock * clock : Clock::s_baseClocks )
	{
		clock->Update( Time::DELTA_SECONDS );
	}
}


//-------------------------------------------------------------------------------------------------
void Engine::Render( ) const
{
	g_ProfilerSystem->StartSample( "RENDER ENGINE" );

	EventSystem::TriggerEvent( ENGINE_RENDER_EVENT );
	g_UISystem->Render( );
	g_DebugSystem->Render( );
	g_ConsoleSystem->Render( );

	//Double Buffer | Also known as FlipAndPresent
	SwapBuffers( g_displayDeviceContext );

	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void Engine::CreateOpenGLWindow( HINSTANCE applicationInstanceHandle )
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>( WindowsMessageHandlingProcedure ); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );
	
	//Original
	//const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	//const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	DWORD windowStyleFlags;
	DWORD windowStyleExFlags;

	//Fullscreen
	// 	if ( TheApp::FULLSCREEN )
	// 	{
	// 		windowStyleFlags = WS_POPUP;
	// 		windowStyleExFlags = WS_EX_APPWINDOW | WS_EX_TOPMOST;
	// 	}
	//Windowed
	//	else
	//{
		windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
		windowStyleExFlags = WS_EX_APPWINDOW;
	//}

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow( );
	GetClientRect( desktopWindowHandle, &desktopRect );

	CalculateLargestWindow( );

	RECT windowRect = { m_offsetXFromWindowsDesktop,
		m_offsetYFromWindowsDesktop,
		m_offsetXFromWindowsDesktop + m_windowPhysicalWidth,
		m_offsetYFromWindowsDesktop + m_windowPhysicalHeight };
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP( ), 0, APP_NAME, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	m_windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL );

	ShowWindow( m_windowHandle, SW_SHOW );
	SetForegroundWindow( m_windowHandle );
	SetFocus( m_windowHandle );

	g_displayDeviceContext = GetDC( m_windowHandle );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize = sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat( g_displayDeviceContext, &pixelFormatDescriptor );
	SetPixelFormat( g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor );
	g_openGLRenderingContext = wglCreateContext( g_displayDeviceContext );
	wglMakeCurrent( g_displayDeviceContext, g_openGLRenderingContext );
}


//-------------------------------------------------------------------------------------------------
void Engine::CalculateLargestWindow( )
{
	Vector2i desktopDimensions = GetDesktopDimensions( );
	float workingSpaceWidth = (float) desktopDimensions.x * PERCENT_OF_SCREEN;
	float workingSpaceHeight = (float) desktopDimensions.y * PERCENT_OF_SCREEN;
	float workingAspect = workingSpaceWidth / workingSpaceHeight;
	float aspect = GetWindowAspectRatio( );
	
	//Width is largest
	if( workingAspect > aspect )
	{
		m_windowPhysicalHeight = (int) workingSpaceHeight;
		m_windowPhysicalWidth = (int) ( workingSpaceHeight * aspect );
	}

	//Height is largest
	else
	{
		m_windowPhysicalWidth = (int) workingSpaceWidth;
		m_windowPhysicalHeight = (int) ( workingSpaceWidth / aspect );
	}

	m_offsetXFromWindowsDesktop = (int) ( ( desktopDimensions.x - m_windowPhysicalWidth ) / 2.f );
	m_offsetYFromWindowsDesktop = (int) ( ( desktopDimensions.y - m_windowPhysicalHeight ) / 2.f );
}


//-------------------------------------------------------------------------------------------------
void Engine::SetTargetFPS( double fps )
{
	m_targetFPS = fps;
}


//-------------------------------------------------------------------------------------------------
void Engine::SetWindowHandle( HWND & handle )
{
	m_windowHandle = handle;
}


//-------------------------------------------------------------------------------------------------
void Engine::SetFullscreen( bool isFullscreen )
{
	if( m_isFullscreen != isFullscreen )
	{
		m_isFullscreen = isFullscreen;
	}
}


//-------------------------------------------------------------------------------------------------
void Engine::IncrementDrawCalls( )
{
	m_currentDrawCalls++;
}


//-------------------------------------------------------------------------------------------------
int Engine::GetCurrentDrawCalls( ) const
{
	return m_currentDrawCalls;
}


//-------------------------------------------------------------------------------------------------
HWND Engine::GetWindowHandle( ) const
{
	return m_windowHandle;
}


//-------------------------------------------------------------------------------------------------
double Engine::GetTargetFPS( ) const
{
	return m_targetFPS;
}


//-------------------------------------------------------------------------------------------------
void Engine::ConsolePrint( std::string const & consoleLog, Color const & color /*= Rgba::WHITE */ )
{
	g_ConsoleSystem->AddLog( consoleLog, color );
}


//-----------------------------------------------------------------------------------------------
// Function By: Squirrel Eiserloh (from StringUtils.cpp)
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;
void Engine::ConsolePrintf( const char* format, ... )
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	ConsolePrint( std::string( textLiteral ), Color::WHITE );
}


//-----------------------------------------------------------------------------------------------
// Can't put color at the end of the list
void Engine::ConsolePrintf( Color const & color, const char* format, ... )
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	ConsolePrint( std::string( textLiteral ), color );
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2i Engine::GetWindowDimensions( )
{
	if( g_EngineSystem )
	{
		return Vector2i( g_EngineSystem->m_windowPhysicalWidth, g_EngineSystem->m_windowPhysicalHeight );
	}
	else
	{
		return Vector2i( Engine::START_WINDOW_WIDTH, Engine::START_WINDOW_HEIGHT );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2i Engine::GetDesktopDimensions( )
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow( );
	// Get the size of screen to the variable desktop
	GetWindowRect( hDesktop, &desktop );
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	int horizontal = desktop.right;
	int vertical = desktop.bottom;

	return Vector2i( horizontal, vertical );
}


//-------------------------------------------------------------------------------------------------
STATIC float Engine::GetWindowAspectRatio( )
{
	return ASPECT_RATIO;
}


//-------------------------------------------------------------------------------------------------
STATIC Vector3f Engine::ScreenToClipSpace( Vector2f const & screenPosition )
{
	Vector2i windowDimensions = GetWindowDimensions( );
	float aspectRatio = (float) windowDimensions.x / (float) windowDimensions.y;
	float xPosition = screenPosition.x / ( float ) windowDimensions.x;
	xPosition = ( xPosition * 2.f - 1.f )*( aspectRatio );
	float yPosition = screenPosition.y / ( float ) windowDimensions.y;
	yPosition = ( yPosition * 2.f - 1.f );
	return Vector3f( xPosition, yPosition, 0.f );
}