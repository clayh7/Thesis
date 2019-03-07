#include "Engine/DebugSystem/Console.hpp"

#include "Engine/InputSystem/Input.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/Logger.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/Net/RCS/RemoteCommandServer.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/RenderState.hpp"
#include "Engine/RenderSystem/Mesh.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
Console * g_ConsoleSystem = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC float Console::OPEN_SPEED = 3.f;
STATIC float Console::BLINK_SPEED = 2.f;
STATIC Vector2f Console::CONSOLE_BORDER( 5.f, 5.f );
STATIC float Console::CONSOLE_LINE_BOTTOM = 20.f;
STATIC float Console::CONSOLE_LINE_TO_BOX_GAP_HEIGHT = 35.f;
STATIC float Console::CONSOLE_HEIGHT = 450.f;
STATIC float Console::CONSOLE_LINE_HEIGHT = 25.f;
STATIC float Console::CONSOLE_LEFT_PADDING = 5.f;
STATIC int Console::NUM_LOGS_TO_VIEW = 16;

STATIC Color Console::GOOD;
STATIC Color Console::BAD;
STATIC Color Console::DEFAULT;
STATIC Color Console::INFO;
STATIC Color Console::REMOTE;

STATIC bool Console::s_serverEchoEnabled = false;

STATIC std::vector<TextRenderer*, UntrackedAllocator<TextRenderer*>> Console::s_consoleTextRenderers;
STATIC std::vector<ConsoleLog*, UntrackedAllocator<ConsoleLog*>> Console::s_consoleLogs;


//-------------------------------------------------------------------------------------------------
void HelpCommand( Command const & )
{
	g_ConsoleSystem->ShowHelp( );
}


//-------------------------------------------------------------------------------------------------
void ClearCommand( Command const & )
{
	g_ConsoleSystem->ClearConsoleLogs( );
}


//-------------------------------------------------------------------------------------------------
void QuitCommand( Command const & )
{
	g_ConsoleSystem->AddLog( "Quiting...", Color::GREEN );
	g_isQuitting = true;
}


//-------------------------------------------------------------------------------------------------
void LogCommand( Command const & command )
{
	std::string defaultArg = "log.txt";
	std::string fileName = command.GetArg( 0, defaultArg );
	std::string const logFilePath = Stringf( "Data/Logs/%s", &fileName[0] );
	std::string const logFileBuffer = g_ConsoleSystem->BuildLogFile( );
	SaveBufferToBinaryFile( logFilePath, logFileBuffer );
	g_ConsoleSystem->AddLog( Stringf("Printed (%d) logs to file: %s", g_ConsoleSystem->GetLogSize(), &logFilePath[0]), Color::GREEN );
}


//-------------------------------------------------------------------------------------------------
void ShadowCommand( Command const & command )
{
	int arg0 = command.GetArg( 0, 0 );
	unsigned char amount = (unsigned char)Clamp( arg0, 0, 255 );
	g_ConsoleSystem->SetShadow( amount );
}


//-------------------------------------------------------------------------------------------------
void ServerEchoCommand( Command const & )
{
	Console::s_serverEchoEnabled = !Console::s_serverEchoEnabled;
	g_ConsoleSystem->AddLog( ( Console::s_serverEchoEnabled ) ? "Echo enabled." : "Echo disabled.", Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
ConsoleLog::ConsoleLog( std::string const &log, Color const &color )
	: m_log( log )
	, m_color( color )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
ConsoleLog::~ConsoleLog( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void SetupColor( )
{
	//Static initialization didn't always work because Color names are set at static initialization as well
	Console::GOOD		= Color::GREEN;
	Console::BAD		= Color::RED;
	Console::DEFAULT	= Color::WHITE;
	Console::INFO		= Color::BLUE;
	Console::REMOTE		= Color::ORANGE;
}


//-------------------------------------------------------------------------------------------------
STATIC void Console::Startup( )
{
	if( g_ConsoleSystem == nullptr )
	{
		g_ConsoleSystem = new Console( );

		//Register initial commands
		RegisterCommand( "help", HelpCommand, " : Show all available commands." );
		RegisterCommand( "clear", ClearCommand, " : Clears console and all previous logs." );
		RegisterCommand( "quit", QuitCommand, " : Closes current application." );
		RegisterCommand( "log", LogCommand, " [filename] : Print all console logs to Data/Logs/[filename]. Default = log.txt" );
		RegisterCommand( "shadow", ShadowCommand, " [0-255] : Change the drop shadow alpha to [0-255]. Default = 0" );
		RegisterCommand( "server_echo", ServerEchoCommand, " : Toggles sending all console lines to all network connections." );

		//Register Events
		EventSystem::RegisterEvent( Input::CHAR_TYPED_EVENT, g_ConsoleSystem, &Console::OnAddChar );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void Console::Shutdown( )
{
	if( g_ConsoleSystem != nullptr )
	{
		delete g_ConsoleSystem;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void Console::RegisterCommand( std::string const & commandName, CommandCallback * callback, std::string const & commandDescription )
{
	if( g_ConsoleSystem == nullptr )
	{
		Console::Startup( );
	}

	g_ConsoleSystem->Register( commandName, callback, commandDescription );
}


//-------------------------------------------------------------------------------------------------
Console::Console( )
	: m_open( false )
	, m_openAmount( 0.f )
	, m_currentLog( -1 )
	, m_logCount( 0 )
	, m_blinkTimer( 0.f )
	, m_showCursor( false )
	, m_shiftActive( false )
	, m_shadowAmount( 255 )
	, m_consoleLine( "" )
	, m_consoleLineTextRenderer( nullptr )
	, m_consoleBox( nullptr )
	, m_consoleBoxBottom( nullptr )
{
	//Setup Colors
	SetupColor( );

	//Setup Basic MeshRenderers
	m_consoleLineTextRenderer = new TextRenderer( "Console Text", Vector2f::ZERO );
	m_consoleLineTextRenderer->SetColor( Console::DEFAULT );
	m_consoleBox = new MeshRenderer( eMeshShape_QUAD, Transform( Vector3f( 0.f, -0.5f, 0.f ), Matrix4f::IDENTITY, Vector3f( 2.f, 1.f, 1.f ) ), RenderState::BASIC_2D );
	m_consoleBox->SetUniform( "uColor", Color( 0, 0, 0, 50 ) );
	m_consoleBoxBottom = new MeshRenderer( eMeshShape_QUAD, Transform( Vector3f( 0.f, -0.5f, 0.f ), Matrix4f::IDENTITY, Vector3f( 2.f, 1.f, 1.f ) ), RenderState::BASIC_2D );
	m_consoleBoxBottom->SetUniform( "uColor", Color( 0, 0, 0, 50 ) );
}


//-------------------------------------------------------------------------------------------------
Console::~Console( )
{
	EventSystem::Unregister( this );

	delete m_consoleBoxBottom;
	m_consoleBoxBottom = nullptr;

	delete m_consoleBox;
	m_consoleBox = nullptr;

	delete m_consoleLineTextRenderer;
	m_consoleLineTextRenderer = nullptr;

	//Delete Statics
	for( ConsoleLog * log : s_consoleLogs )
	{
		delete log;
		log = nullptr;
	}
	s_consoleLogs.clear( );

	for( TextRenderer * textRenderer : s_consoleTextRenderers )
	{
		delete textRenderer;
		textRenderer = nullptr;
	}
	s_consoleTextRenderers.clear( );

	for( auto command : m_commands )
	{
		delete command.second;
		command.second = nullptr;
	}
	m_commands.clear( );
	
	for( auto description : m_commandDescriptions )
	{
		delete description.second;
		description.second = nullptr;
	}
	m_commandDescriptions.clear( );
}


//-------------------------------------------------------------------------------------------------
void Console::Update( )
{
	//Open/Close Console
	if( g_InputSystem->WasKeyJustPressed( Input::KEY_TILDE ) )
	{
		ToggleOpen( );
	}

	//Opening Effect Timer
	if( m_open )
	{
		m_openAmount += Time::DELTA_SECONDS * OPEN_SPEED;
		m_blinkTimer += Time::DELTA_SECONDS * BLINK_SPEED;
		if ( m_blinkTimer > 1.f )
		{
			m_blinkTimer = 0.f;
			m_showCursor = !m_showCursor;
		}
	}
	else
	{
		m_openAmount -= Time::DELTA_SECONDS * OPEN_SPEED;
	}

	m_openAmount = Clamp( m_openAmount, 0.f, 1.f );

	if( m_openAmount <= 0.f && !m_open )
	{
		return;
	}

	//Update current line mesh
	if( m_showCursor )
	{
		m_consoleLineTextRenderer->SetText( m_consoleLine + "|" );
	}
	else
	{
		m_consoleLineTextRenderer->SetText( m_consoleLine );
	}

	//Update console box
	float boxPosition = Lerp( -1.5f, -.5f, m_openAmount );
	m_consoleBox->SetPosition( Vector3f( 0.f, boxPosition, 0.f ) );
	m_consoleBox->Update( true );

	m_consoleBoxBottom->SetPosition( Vector3f( 0.f, boxPosition - 0.915f, 0.f ) );
	m_consoleBoxBottom->Update( true );

	//Update console line
	float linePosition = Lerp( CONSOLE_LINE_BOTTOM - CONSOLE_HEIGHT, CONSOLE_LINE_BOTTOM, m_openAmount );
	m_consoleLineTextRenderer->SetPosition( Vector2f( CONSOLE_LEFT_PADDING, linePosition ) );
	m_consoleLineTextRenderer->Update( );

	//Update logs
	for( unsigned int logIndex = 0; logIndex < s_consoleTextRenderers.size( ); ++logIndex )
	{
		float logPositionYOffset = CONSOLE_LINE_HEIGHT * ( m_currentLog - logIndex );
		Vector2f logPosition = Vector2f( CONSOLE_LEFT_PADDING, linePosition + CONSOLE_LINE_TO_BOX_GAP_HEIGHT + logPositionYOffset );

		s_consoleTextRenderers[logIndex]->SetColor( s_consoleLogs[logIndex]->m_color );
		s_consoleTextRenderers[logIndex]->SetPosition( logPosition );
		s_consoleTextRenderers[logIndex]->Update( );
	}
}


//-------------------------------------------------------------------------------------------------
void Console::Render( ) const
{
	if( m_openAmount <= 0.f )
	{
		return;
	}

	//Render console box
	g_RenderSystem->MeshRender( m_consoleBox );
	g_RenderSystem->MeshRender( m_consoleBoxBottom );
	m_consoleLineTextRenderer->Render( );

	//Render the lines in the box
	for( int logIndex = 0; logIndex < (int) s_consoleTextRenderers.size( ); ++logIndex )
	{
		if ( logIndex <= m_currentLog && logIndex > m_currentLog - NUM_LOGS_TO_VIEW )
		{
			s_consoleTextRenderers[logIndex]->Render( );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Console::Register( std::string const & commandName, CommandCallback * callback, std::string const & commandDescription )
{
	//if it's a new command
	size_t commandHash = std::hash<std::string>{ }( commandName );
	auto foundCommand = m_commands.find( commandHash );
	if( foundCommand == m_commands.end( ) )
	{
		//Make new command
		CommandStaticFunction * registerCommand = new CommandStaticFunction( );
		registerCommand->m_function = callback;
		m_commands.insert( std::pair<size_t, CommandBase*>( commandHash, registerCommand ) );

		//Add command description
		std::string fullDescription = Stringf( "%s%s", commandName.c_str( ), commandDescription.c_str( ) );
		char * descriptionString = CreateNewCString( fullDescription );
		m_commandDescriptions.insert( std::pair<std::string, char*>( commandName, descriptionString ) );
	}
}


//-------------------------------------------------------------------------------------------------
void Console::RegisterCommandEvent( std::string const & commandName, EventCallback * callback, std::string const & commandDescription )
{
	//Make new command
	EventStaticFunction * registerCommand = new EventStaticFunction( );
	registerCommand->m_event = callback;

	//Add new command
	size_t commandHash = std::hash<std::string>{ }( commandName );
	m_commands.insert( std::pair<size_t, CommandBase*>( commandHash, registerCommand ) );

	//Add command description
	std::string fullDescription = Stringf( "%s%s", commandName.c_str( ), commandDescription.c_str( ) );
	char * descriptionString = CreateNewCString( fullDescription );
	m_commandDescriptions.insert( std::pair<std::string, char*>( commandName, descriptionString ) );
}


//-------------------------------------------------------------------------------------------------
void Console::RunCommand( std::string const & commandString, bool remote /*= false*/ )
{
	//Remote commands are commands send over the network
	if( remote )
	{
		std::string remoteCommandString = Stringf( "REMOTE COMMAND: %s", commandString.c_str( ) );
		AddLog( remoteCommandString, REMOTE, remote, false );
	}
	else
	{
		AddLog( commandString, DEFAULT, remote, false );
	}

	if( LoggingSystem )
	{
		LoggingSystem->LogPrintf( "Input: %s", commandString.c_str( ) );
	}

	Command command( commandString );
	size_t commandHash = std::hash<std::string>{ }( command.GetName( ) );
	auto foundCommandIter = m_commands.find( commandHash );
	if( foundCommandIter != m_commands.end( ) )
	{
		//Execute command if registered
		foundCommandIter->second->Execute( command );
	}
	else
	{
		AddLog( "Invalid Command. Type 'help' for a list of commands.", BAD, remote );
	}
}


//-------------------------------------------------------------------------------------------------
void Console::AddLog( std::string const & log, Color const & color /*= DEFAULT*/, bool remote /*= false*/, bool debug /*= true*/ )
{
	//Add the new log
	s_consoleLogs.push_back( new ConsoleLog( log, color ) );
	TextRenderer * logTextRenderer = new TextRenderer( log, Vector2f::ZERO );
	s_consoleTextRenderers.push_back( logTextRenderer );

	//Send to other network connections
	if( !remote )
	{
		if( s_serverEchoEnabled )
		{
			RemoteCommandServer::Send( eRCSMessageType_ECHO, log );
		}
	}

	//Add Console log to logging system
	if( debug )
	{
		if( LoggingSystem )
		{
			LoggingSystem->LogPrintf( "%s", log.c_str( ) );
		}
	}

	//If you're looking at the most recent log, have the logs shift up
	if ( m_currentLog == m_logCount - 1 )
	{
		++m_currentLog;
	}
	++m_logCount;
}


//-------------------------------------------------------------------------------------------------
void Console::ClearConsoleLogs( )
{
	//Clear Statics
	for( ConsoleLog * log : s_consoleLogs )
	{
		delete log;
		log = nullptr;
	}
	s_consoleLogs.clear( );

	for( TextRenderer * textRenderer : s_consoleTextRenderers )
	{
		delete textRenderer;
		textRenderer = nullptr;
	}
	s_consoleTextRenderers.clear( );

	m_currentLog = -1;
	m_logCount = 0;
}


//-------------------------------------------------------------------------------------------------
std::string const Console::BuildLogFile( )
{
	std::string logFile = "";
	std::string newLine = "\r\n";
	for( int logIndex = 0; logIndex < m_logCount; ++logIndex )
	{
		logFile.append( s_consoleLogs[logIndex]->m_log );
		logFile.append( newLine );
	}
	return logFile;
}


//-------------------------------------------------------------------------------------------------
void Console::ShowHelp( )
{
	AddLog( "Showing Registered Commands", Color::GREEN );
	auto commandIter = m_commandDescriptions.begin( );
	while( commandIter != m_commandDescriptions.end( ) )
	{
		AddLog( commandIter->second, Color::WHITE );
		++commandIter;
	}
}


//-------------------------------------------------------------------------------------------------
void Console::PressKey( unsigned char asKey )
{
	if( asKey == Input::KEY_TILDE )
	{
		ToggleOpen( );
	}
	else if( asKey == Input::KEY_BACKSPACE )
	{
		RemoveLastChar( );
	}
	else if( asKey == Input::KEY_ESCAPE )
	{
		if( m_consoleLine.length( ) > 0 )
		{
			ClearConsoleLine( );
		}
		else
		{
			ToggleOpen( );
		}
	}
	else if( asKey == Input::KEY_ENTER )
	{
		if( m_consoleLine.length( ) > 0 )
		{
			RunCommand( m_consoleLine );
			ClearConsoleLine( );
		}
		else
		{
			ToggleOpen( );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Console::ReleaseKey( unsigned char )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void Console::MoveMouseWheel( int wheelDelta )
{
	if( wheelDelta > 0 )
	{
		//Moving up
		m_currentLog--;
	}
	else
	{
		//Moving down
		m_currentLog++;
	}
	m_currentLog = Clamp( m_currentLog, 0, m_logCount-1 );
}


//-------------------------------------------------------------------------------------------------
void Console::OnAddChar( NamedProperties & charTypedEvent )
{
	if( m_open )
	{
		//Get character
		unsigned char asKey;
		charTypedEvent.Get( "asKey", asKey );

		AddChar( asKey );
	}
}


//-------------------------------------------------------------------------------------------------
void Console::SetShadow( unsigned char shadowAmount )
{
	m_shadowAmount = shadowAmount;
	AddLog( Stringf( "Setting Drop Shadow to: %d", ( int ) shadowAmount ), Color::GREEN );
}


//-------------------------------------------------------------------------------------------------
int Console::GetLogSize( ) const
{
	return m_logCount;
}


//-------------------------------------------------------------------------------------------------
void Console::ToggleOpen( )
{
	m_open = !m_open;
}


//-------------------------------------------------------------------------------------------------
void Console::AddChar( unsigned char newChar )
{
	m_consoleLine.push_back( newChar );
}


//-------------------------------------------------------------------------------------------------
void Console::RemoveLastChar( )
{
	if( m_consoleLine.size( ) > 0 )
	{
		m_consoleLine.pop_back( );
	}
}


//-------------------------------------------------------------------------------------------------
void Console::ClearConsoleLine( )
{
	m_consoleLine = "";
}