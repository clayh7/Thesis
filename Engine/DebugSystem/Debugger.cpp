#include "Engine/DebugSystem/Debugger.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/Logger.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/Memory/MemoryAnalytics.hpp"
#include "Engine/Utils/NetworkUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
Debugger * g_DebugSystem = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC float const Debugger::DEBUG_LINE_SPACING = 25.f;


//-------------------------------------------------------------------------------------------------
void DebugFPSCommand( Command const & )
{
	g_ConsoleSystem->AddLog( "Toggling FPS debug.", Console::GOOD );
	g_DebugSystem->ToggleDebugFPS( );
}


//-------------------------------------------------------------------------------------------------
void DebugUnitCommand( Command const & )
{
	g_ConsoleSystem->AddLog( "Toggling unit debug.", Console::GOOD );
	g_DebugSystem->ToggleDebugUnit( );
}


//-------------------------------------------------------------------------------------------------
void DebugMemoryCommand( Command const & )
{
	g_ConsoleSystem->AddLog( "Toggling memory debug.", Console::GOOD );
	g_DebugSystem->ToggleDebugMemory( );
}


//-------------------------------------------------------------------------------------------------
void DebugFlushCommand( Command const & )
{
	g_ConsoleSystem->AddLog( "Flushing memory callstack.", Console::GOOD );
	g_DebugSystem->DebugFlushMemory( );
}


//-------------------------------------------------------------------------------------------------
Debugger::Debugger( )
	: m_showFPSDebug( false )
	, m_showUnitDebug( false )
	, m_showMemoryDebug( false )
	, m_lineCount( 14 )
{
	g_ConsoleSystem->RegisterCommand( "debug_fps", &DebugFPSCommand, " : Show/Hide FPS info." );
	g_ConsoleSystem->RegisterCommand( "debug_unit", &DebugUnitCommand, " : Show/Hide frame breakdown info." );

	Vector2i windowDimensions = GetWindowDimensions( );
	float topOfWindow = static_cast<float>( windowDimensions.y - 15.f );

	for( int index = 0; index < m_lineCount; ++index )
	{
		Vector2f linePosition = Vector2f( 10.f, topOfWindow - DEBUG_LINE_SPACING * index );
		m_debugTexts.push_back( new TextRenderer( " ", linePosition ) );
	}

	//LoggingSystem = new Logger( );
	//LoggingSystem->Begin( );

	g_ProfilerSystem = new BProfiler( );

#ifdef MEMORY_TRACKING
	g_ConsoleSystem->RegisterCommand( "debug_memory", &DebugMemoryCommand, " : Show/Hide memory allocation info." );
	g_ConsoleSystem->RegisterCommand( "debug_flush", &DebugFlushCommand, " : Print memory callstack to the debug log." );
#endif // MEMORY_TRACKING
}


//-------------------------------------------------------------------------------------------------
Debugger::~Debugger( )
{
	for( TextRenderer * textRenderer : m_debugTexts )
	{
		delete textRenderer;
		textRenderer = nullptr;
	}
	m_debugTexts.clear( );

	delete g_ProfilerSystem;
	g_ProfilerSystem = nullptr;

	delete LoggingSystem;
	LoggingSystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
void Debugger::Update( )
{
	int currentLine = 0;

	if( m_showFPSDebug )
	{
		UpdateTextFPS( currentLine );
	}

	//#TODO: Currently handled by the Profiler, may update this later to make it like UE4
	if( m_showUnitDebug )
	{
		//UpdateTextUnit( currentLine );
	}

#ifdef MEMORY_TRACKING
	if( m_showMemoryDebug )
	{
		UpdateTextMemory( currentLine );
	}
#endif // MEMORY_TRACKING

#if MEMORY_TRACKING == 1
	if( m_showMemoryDebug )
	{
		UpdateTextMemoryVerbose( currentLine );
	}
#endif // MEMORY_TRACKING == 1

	ClearTextRemaining( currentLine );
}


//-------------------------------------------------------------------------------------------------
void Debugger::UpdateTextFPS( int & currentLine )
{
	float fps = 1.f / Time::DELTA_SECONDS;
	std::string debugText = Stringf( "FPS: %.1f", fps );
	//debugText = Stringf( "DRAW: %d", g_EngineSystem->GetCurrentDrawCalls( ) );
	Color fpsColor = Color::WHITE;
	if( fps < 30.0f )
	{
		fpsColor = Color::RED;
	}
	else if( fps < 45.0f )
	{
		fpsColor = Color::ORANGE;
	}
	else if( fps < 55.0f )
	{
		fpsColor = Color::YELLOW;
	}
	m_debugTexts[currentLine]->SetText( debugText );
	m_debugTexts[currentLine]->SetColor( fpsColor );
	m_debugTexts[currentLine]->Update( );
	currentLine += 1;
}


//-------------------------------------------------------------------------------------------------
void Debugger::UpdateTextUnit( int & currentLine )
{
	float gameTime = 2.0f;
	std::string debugText = Stringf( "Game: %.1fms", gameTime );
	Color unitColor = Color::WHITE;
	if( gameTime > 16.0f )
	{
		unitColor = Color::RED;
	}
	else if( gameTime > 8.0f )
	{
		unitColor = Color::YELLOW;
	}
	m_debugTexts[currentLine]->SetText( debugText );
	m_debugTexts[currentLine]->SetColor( unitColor );
	m_debugTexts[currentLine]->Update( );
	currentLine += 1;

	float renderTime = 2.5f;
	debugText = Stringf( "Render: %.1fms", renderTime );
	unitColor = Color::WHITE;
	if( gameTime > 16.0f )
	{
		unitColor = Color::RED;
	}
	else if( gameTime > 8.0f )
	{
		unitColor = Color::YELLOW;
	}
	m_debugTexts[currentLine]->SetText( debugText );
	m_debugTexts[currentLine]->SetColor( unitColor );
	m_debugTexts[currentLine]->Update( );
	currentLine += 1;

	debugText = Stringf( "Draw: %d", g_EngineSystem->GetCurrentDrawCalls( ) );
	m_debugTexts[currentLine]->SetText( debugText );
	m_debugTexts[currentLine]->SetColor( Color::WHITE );
	m_debugTexts[currentLine]->Update( );
	currentLine += 1;
}


//-------------------------------------------------------------------------------------------------
void Debugger::UpdateTextMemory( int & currentLine )
{
	std::string debugText = g_MemoryAnalyticsSystem->GetMemoryAllocationsString( );
	m_debugTexts[currentLine]->SetText( debugText );
	m_debugTexts[currentLine]->SetColor( Color::WHITE );
	m_debugTexts[currentLine]->Update( );
	currentLine += 1;

	debugText = g_MemoryAnalyticsSystem->GetMemoryAveragesString( );
	m_debugTexts[currentLine]->SetText( debugText );
	m_debugTexts[currentLine]->SetColor( Color::WHITE );
	m_debugTexts[currentLine]->Update( );
	currentLine += 1;
}


//-------------------------------------------------------------------------------------------------
void Debugger::UpdateTextMemoryVerbose( int & currentLine )
{
	//Get the stats
	UntrackedCallstackStatsMap & callstackStatsMap = g_MemoryAnalyticsSystem->m_callstackStatsMap;

	//Order them base on allocation size
	std::map<size_t, CallstackStats> orderedStats;
	for( auto callstackStatsItem : callstackStatsMap )
	{
		orderedStats.insert( std::pair<size_t, CallstackStats>( callstackStatsItem.second.totalBytes, callstackStatsItem.second ) );
	}

	for( auto callstackStatsIter = orderedStats.end( ); callstackStatsIter != orderedStats.begin( ); )
	{
		--callstackStatsIter;
		if( currentLine + 2 <= m_lineCount )
		{
			CallstackStats & callstackStats = callstackStatsIter->second;
			std::string stats = Stringf( "Allocations: %u | Bytes: %u", callstackStats.totalAllocations, callstackStats.totalBytes );
			std::string line = callstackStats.lineAndNumber;
			m_debugTexts[currentLine]->SetText( stats );
			m_debugTexts[currentLine]->SetColor( Color::WHITE );
			m_debugTexts[currentLine]->Update( );
			currentLine += 1;

			m_debugTexts[currentLine]->SetText( line );
			m_debugTexts[currentLine]->SetColor( Color::WHITE );
			m_debugTexts[currentLine]->Update( );
			currentLine += 1;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Debugger::ClearTextRemaining( int & currentLine )
{
	//Clear the rest
	while( currentLine < m_lineCount )
	{
		m_debugTexts[currentLine]->SetText( " " );
		m_debugTexts[currentLine]->Update( );
		currentLine += 1;
	}
}


//-------------------------------------------------------------------------------------------------
//#TODO: Make the Debug renders a single draw call each
void Debugger::Render( ) const
{
	if( m_showFPSDebug || m_showUnitDebug || m_showMemoryDebug )
	{
		for( size_t memDebugIndex = 0; memDebugIndex < m_debugTexts.size( ); ++memDebugIndex )
		{
			m_debugTexts[memDebugIndex]->Render( );
		}
	}

	g_ProfilerSystem->Render( );
}


//-------------------------------------------------------------------------------------------------
void Debugger::ToggleDebugFPS( )
{
	m_showFPSDebug = !m_showFPSDebug;
}


//-------------------------------------------------------------------------------------------------
void Debugger::ToggleDebugUnit( )
{
	m_showUnitDebug = !m_showUnitDebug;
}


//-------------------------------------------------------------------------------------------------
void Debugger::ToggleDebugMemory( )
{
	m_showMemoryDebug = !m_showMemoryDebug;
}


//-------------------------------------------------------------------------------------------------
void Debugger::DebugFlushMemory( )
{
	g_MemoryAnalyticsSystem->Flush( );
}