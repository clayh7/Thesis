#include "Game/General/App.hpp"

#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Net/NetworkSystem.hpp"
#include "Engine/Net/RCS/RemoteCommandServer.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/Framebuffer.hpp"
#include "Engine/RenderSystem/Mesh.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Game/General/GameCommon.hpp"
#include "Game/General/Game.hpp"


//-------------------------------------------------------------------------------------------------
App* g_AppSystem = nullptr;


//-------------------------------------------------------------------------------------------------
App::App( )
	: m_fbo( nullptr )
	, m_screenMesh( nullptr )
	, m_screenMaterial( nullptr )
	, m_screenEffect( nullptr )
{
	//Startup text
	g_ConsoleSystem->AddLog( "Engine: Bread v0.6.0", Console::INFO );
	g_ConsoleSystem->AddLog( Stringf( "Project: %s", Engine::APP_NAME ), Console::INFO );
	g_ConsoleSystem->AddLog( "Author: Clay Howell", Console::INFO );

	//Start server with self (for quick debugging)
	//StartClientServer( );

	//Create Game
	g_GameSystem = new Game();

	//Setup mouse appearance
	//InputSystem->HoldMousePosition( MOUSE_SNAP_POSITION );
	g_InputSystem->SetMouseHidden( false );

	std::vector<TextureFormat> textureFormats; //#TODO: Why exactly is this an array again?
	textureFormats.push_back( TextureFormat_RGBA8 );

	//Create Nothing Shader PostProcessFX
	Vector2i windowDimensions = GetWindowDimensions( );
	m_fbo = new Framebuffer( windowDimensions.x, windowDimensions.y, textureFormats, TextureFormat_D24S8 );
	m_screenMesh = Mesh::GetMeshShape( eMeshShape_QUAD );
	m_screenMaterial = new Material( "Data/Shaders/post.vert", "Data/Shaders/postNothing.frag" );
	m_screenEffect = new MeshRenderer( m_screenMesh, m_screenMaterial, Transform( Vector3f::ZERO, Matrix4f::IDENTITY, 2.f ) );
	m_screenEffect->SetUniform( "uDiffuseTex", m_fbo->GetColorTexture( 0 ) );
}


//-------------------------------------------------------------------------------------------------
App::~App( )
{
	delete g_GameSystem;
	g_GameSystem = nullptr;

	delete m_screenEffect;
	m_screenEffect = nullptr;

	delete m_screenMaterial;
	m_screenMaterial = nullptr;

	delete m_fbo;
	m_fbo = nullptr;
}


//-------------------------------------------------------------------------------------------------
void App::Update( )
{
	//Menu Inputs / Mouse Inputs
	UpdateInputs( );

	g_GameSystem->Update( );
}


//-------------------------------------------------------------------------------------------------
void App::UpdateInputs( )
{
	//App gains focus
	if ( g_InputSystem->GainedFocus( ) )
	{
		//InputSystem->HoldMousePosition( MOUSE_SNAP_POSITION );
		g_InputSystem->SetMouseHidden( true );
		//Time::DELTA_SECONDS = 0.f;
	}

	//Freeze game while not focused
	if ( !g_InputSystem->IsFocused( ) )
	{
		//Time::DELTA_SECONDS = 0.f;
	}

	//App loses focus
	if ( g_InputSystem->LostFocus( ) )
	{
		//InputSystem->ReleaseMouseHold( );
		g_InputSystem->SetMouseHidden( false );
	}

	//Quitting App
	if ( g_InputSystem->WasKeyJustPressed( Input::KEY_ESCAPE ) )
	{
		//g_isQuitting = true;
	}
}


//-------------------------------------------------------------------------------------------------
void App::Render( ) const
{
	g_ProfilerSystem->StartSample( "RENDER GAME" );

	g_RenderSystem->BindFramebuffer( m_fbo );
	g_RenderSystem->ClearScreen( Color::BLACK );

	//Draw Game
	g_GameSystem->Render( );

	g_RenderSystem->BindFramebuffer( nullptr );
	g_RenderSystem->MeshRender( m_screenEffect );

	g_ProfilerSystem->StopSample( );
}


//-------------------------------------------------------------------------------------------------
void App::StartClientServer( )
{
	if( !RemoteCommandServer::Join( NetworkUtils::GetLocalHostName( ) ) )
	{
		if(RemoteCommandServer::Host( ) )
		{
			g_ConsoleSystem->AddLog( "Started Hosting", Console::GOOD );
		}
		else
		{
			g_ConsoleSystem->AddLog( "Problems starting Host", Console::BAD );
		}
	}
	else
	{
		g_ConsoleSystem->AddLog( "Connected to Host", Console::GOOD );
	}
}