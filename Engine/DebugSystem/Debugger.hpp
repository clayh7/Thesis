#pragma once

#include <vector>


//-------------------------------------------------------------------------------------------------
class Debugger;
class TextRenderer;
class Command;


//-------------------------------------------------------------------------------------------------
extern Debugger * g_DebugSystem;


//-------------------------------------------------------------------------------------------------
void DebugFPSCommand( Command const & );
void DebugUnitCommand( Command const & );
void DebugMemoryCommand( Command const & );
void DebugFlushCommand( Command const & );


//-------------------------------------------------------------------------------------------------
class Debugger
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static float const DEBUG_LINE_SPACING;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	bool m_showFPSDebug;
	bool m_showUnitDebug;
	bool m_showMemoryDebug;
	int m_lineCount;
	std::vector<TextRenderer*> m_debugTexts;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Debugger( );
	~Debugger( );
	void Update( );
	void UpdateTextFPS( int & currentLine );
	void UpdateTextUnit( int & currentLine );
	void UpdateTextMemory( int & currentLine );
	void UpdateTextMemoryVerbose( int & currentLine );
	void ClearTextRemaining( int & currentLine );
	void Render( ) const;

	void ToggleDebugFPS( );
	void ToggleDebugUnit( );
	void ToggleDebugMemory( );
	void DebugFlushMemory( );
};