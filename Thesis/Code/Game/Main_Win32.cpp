#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Core/Engine.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Game/General/App.hpp"


//-------------------------------------------------------------------------------------------------
void Update( )
{
	g_ProfilerSystem->StartSample( "UPDATE" );

	g_EngineSystem->Update( );
	g_AppSystem->Update( );
	g_EngineSystem->LateUpdate( );

	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void Render( )
{
	g_ProfilerSystem->StartSample( "RENDER" );

	//Clear Screen
	g_RenderSystem->ClearScreen( Color::BLACK );

	//Print world
	g_AppSystem->Render( );
	g_EngineSystem->Render( ); //Needs to run second to print on top of App

	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void RunFrame( )
{
	g_ProfilerSystem->FrameMark( );
	Update( );
	Render( );
}


//-------------------------------------------------------------------------------------------------
void Initialize( HINSTANCE applicationInstanceHandle )
{
	g_EngineSystem = new Engine( applicationInstanceHandle );
	g_AppSystem = new App( );
}


//-------------------------------------------------------------------------------------------------
void Shutdown( )
{
	//Destroy in inverse order
	delete g_AppSystem;
	g_AppSystem = nullptr;

	delete g_EngineSystem;
	g_EngineSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR /*commandLineString*/, int )
{
	Initialize( applicationInstanceHandle );

	while ( !g_isQuitting )
	{
		RunFrame( );
	}

	Shutdown( );
	return 0;
}