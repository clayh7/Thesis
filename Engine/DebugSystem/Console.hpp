#pragma once

#include <map>
#include <string>
#include <vector>
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/DebugSystem/Command.hpp"
#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Memory/MemoryAnalytics.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
typedef void ( EventCallback )( NamedProperties & );
class Console;
class Command;
class Mesh;
class Material;
class MeshRenderer;
class NamedProperties;
class TextRenderer;


//-------------------------------------------------------------------------------------------------
extern Console * g_ConsoleSystem;


//-------------------------------------------------------------------------------------------------
typedef void ( CommandCallback )( Command const & );
void HelpCommand( Command const & );
void ClearCommand( Command const & );
void QuitCommand( Command const & );
void LogCommand( Command const & );
void ServerEchoCommand( Command const & );


//-----------------------------------------------------------------------------------------------
class CommandBase
{
public:
	virtual void Execute( Command & ) const = 0;
};


//-------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_FunctionType>
class CommandObjectFunction : public CommandBase
{
public:
	T_ObjectType * m_object;
	T_FunctionType m_function;

public:
	virtual void Execute( Command & command ) const override
	{
		( m_object->*m_function )( command );
	}
};


//-------------------------------------------------------------------------------------------------
class CommandStaticFunction : public CommandBase
{
public:
	CommandCallback * m_function;

public:
	virtual void Execute( Command & command ) const override
	{
		( m_function )( command );
	}
};


//-------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_FunctionType>
class EventObjectFunction : public CommandBase
{
public:
	T_ObjectType * m_object;
	T_FunctionType m_function;

public:
	virtual void Execute( Command & command ) const override
	{
		NamedProperties commandEvent;
		commandEvent.Set( "Command", command );
		( m_object->*m_function )( commandEvent );
	}
};


//-------------------------------------------------------------------------------------------------
class EventStaticFunction : public CommandBase
{
public:
	EventCallback * m_event;

public:
	virtual void Execute( Command & command ) const override
	{
		NamedProperties commandEvent;
		commandEvent.Set( "Command", command );
		( m_event )( commandEvent );
	}
};


//-------------------------------------------------------------------------------------------------
class ConsoleLog
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	std::string m_log;
	Color m_color;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	ConsoleLog( std::string const & log, Color const & color );
	~ConsoleLog( );
};


//-------------------------------------------------------------------------------------------------
class Console
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static float OPEN_SPEED;
	static float BLINK_SPEED;
	static Vector2f CONSOLE_BORDER;
	static float CONSOLE_LINE_BOTTOM;
	static float CONSOLE_LINE_TO_BOX_GAP_HEIGHT;
	static float CONSOLE_HEIGHT;
	static float CONSOLE_LINE_HEIGHT;
	static float CONSOLE_LEFT_PADDING;
	static int NUM_LOGS_TO_VIEW;

	//Static Memory Allocations
	static std::vector<ConsoleLog*, UntrackedAllocator<ConsoleLog*>> s_consoleLogs;
	static std::vector<TextRenderer*, UntrackedAllocator<TextRenderer*>> s_consoleTextRenderers;

public:
	static Color GOOD;
	static Color BAD;
	static Color DEFAULT;
	static Color INFO;
	static Color REMOTE;

	static bool s_serverEchoEnabled;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	bool m_open;
	float m_openAmount;
	int m_currentLog;
	int m_logCount;
	float m_blinkTimer;
	bool m_showCursor;
	bool m_shiftActive;
	unsigned char m_shadowAmount;
	std::string m_consoleLine;
	TextRenderer * m_consoleLineTextRenderer;
	MeshRenderer * m_consoleBox;
	MeshRenderer * m_consoleBoxBottom;

	std::map<size_t, CommandBase*> m_commands;
	std::map<std::string, char*> m_commandDescriptions;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void Startup( );
	static void Shutdown( );
	static void RegisterCommand( std::string const & commandName, CommandCallback * callback, std::string const & commandDescription );

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Console( );
	~Console( );
	void Update( );
	void Render( ) const;
	void Register( std::string const & commandName, CommandCallback * callback, std::string const & commandDescription );
	void RegisterCommandEvent( std::string const & commandName, EventCallback * callback, std::string const & commandDescription );
	void RunCommand( std::string const & commandString, bool remote = false );
	void AddLog( std::string const & log, Color const & color = DEFAULT, bool remote = false, bool debug = true );
	void ClearConsoleLogs( );
	std::string const BuildLogFile( );

	void ShowHelp( );
	void PressKey( unsigned char asKey );
	void ReleaseKey( unsigned char asKey );
	void MoveMouseWheel( int wheelDelta );
	void SetShadow( unsigned char shadowAmount );

	int GetLogSize( ) const;
	bool IsOpen( ) const { return m_open; }

private:
	void ToggleOpen( );
	void AddChar( unsigned char newChar );
	void RemoveLastChar( );
	void ClearConsoleLine( );
	void OnAddChar( NamedProperties & charTypedEvent );

//-------------------------------------------------------------------------------------------------
// Function Templates
//-------------------------------------------------------------------------------------------------
public:
	template<typename T_ObjectType, typename T_FunctionType>
	void RegisterCommand( std::string const & commandName, T_ObjectType * object, T_FunctionType function, std::string const & commandDescription )
	{
		//Make new command
		CommandObjectFunction<T_ObjectType, T_FunctionType> * registerCommand = new CommandObjectFunction<T_ObjectType, T_FunctionType>( );
		registerCommand->m_object = object;
		registerCommand->m_function = function;

		//Add new command
		size_t commandHash = std::hash<std::string>{ }( commandName );
		m_commands.insert( std::pair<size_t, CommandBase*>( commandHash, registerCommand ) );

		//Add command description
		std::string fullDescription = Stringf( "%s%s", commandName.c_str( ), commandDescription.c_str( ) );
		char * descriptionString = CreateNewCString( fullDescription );
		m_commandDescriptions.insert( std::pair<std::string, char*>( commandName, descriptionString ) );
	}

	template<typename T_ObjectType, typename T_FunctionType>
	void RegisterCommandEvent( std::string const & commandName, T_ObjectType * object, T_FunctionType function, std::string const & commandDescription )
	{
		//Make new command
		EventObjectFunction<T_ObjectType, T_FunctionType> * registerCommand = new EventObjectFunction<T_ObjectType, T_FunctionType>( );
		registerCommand->m_object = object;
		registerCommand->m_function = function;

		//Add new command
		size_t commandHash = std::hash<std::string>{ }( commandName );
		m_commands.insert( std::pair<size_t, CommandBase*>( commandHash, registerCommand ) );

		//Add command description
		std::string fullDescription = Stringf( "%s%s", commandName.c_str( ), commandDescription.c_str( ) );
		char * descriptionString = CreateNewCString( fullDescription );
		m_commandDescriptions.insert( std::pair<std::string, char*>( commandName, descriptionString ) );
	}
};