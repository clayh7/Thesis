#pragma once

//-------------------------------------------------------------------------------------------------
// Link in the fmodex_vc.lib static library
#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" )


//-------------------------------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//-------------------------------------------------------------------------------------------------
class BAudio;


//-------------------------------------------------------------------------------------------------
extern BAudio * g_AudioSystem;


//-------------------------------------------------------------------------------------------------
typedef unsigned int SoundID;
typedef void * AudioChannelHandle;


//-------------------------------------------------------------------------------------------------
class BAudio
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static unsigned int const MISSING_SOUND_ID = 0xffffffff;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	FMOD::System * m_fmodSystem;
	std::map< std::string, SoundID > m_registeredSoundIDs;
	std::vector< FMOD::Sound* > m_registeredSounds;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	BAudio( );
	~BAudio( );

	void Update( ); // Must be called at regular intervals (e.g. every frame)
	SoundID CreateOrGetSound( std::string const & soundFileName );
	AudioChannelHandle PlayAudio( SoundID soundID, float volumeLevel = 1.f, bool looping = false );
	void StopChannel( AudioChannelHandle channel );

private:
	void InitializeFMOD( );
	void ValidateResult( FMOD_RESULT result );
};