#pragma once

#include <map>
#include <vector>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Memory/ObjectPool.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"


//-------------------------------------------------------------------------------------------------
class BitmapFont;
class BProfiler;
class BProfilerReport;
class BProfilerSample;
class Command;


//-------------------------------------------------------------------------------------------------
void PingProfilerCommand( Command const & );
void EnableProfilerCommand( Command const & );
void DisableProfilerCommand( Command const & );


//-------------------------------------------------------------------------------------------------
extern BProfiler * g_ProfilerSystem;


//-------------------------------------------------------------------------------------------------
class BProfiler
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * ROOT_SAMPLE;
	static int const POOL_SIZE = 100000;
	static int const LINE_COUNT = 40;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	bool m_toBeEnabled;
	bool m_toBePinged;
	bool m_enabled;
	bool m_reportList;
	bool m_show;
	BitmapFont * m_profilerFont;
	uint64_t m_previousFrameTotalTime;
	BProfilerSample * m_currentSample;
	BProfilerSample * m_currentSampleSet;
	BProfilerSample * m_previousSampleSet;
	ObjectPool<BProfilerSample> m_samplePool;
	std::vector<TextRenderer*> m_profilerLines;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	BProfiler( );
	~BProfiler( );

	void Render( ) const;
	void GenerateListReport( BProfilerSample * sample, int * index, int depth, std::multimap<int, BProfilerReport> * report ) const;
	void GenerateFlatReport( BProfilerSample * sample, std::multimap<int, BProfilerReport> * report ) const;
	void RenderReport( std::multimap<int, BProfilerReport> * report ) const;

	void SetEnabled( bool isEnabled );
	void SetPing( );
	void ToggleLayout( );
	void StartSample( char const * sampleTag );
	void StopSample( );
	void Delete( BProfilerSample * sample );
	void UpdateInput( );
	void FrameMark( );
	void IncrementNews( );
	void IncrementDeletes( );
	void SetProfilerVisible( bool show );
};