#pragma once

#include "Engine/Math/AABB2f.hpp"

/*
//-------------------------------------------------------------------------------------------------
class Texture;
class SpriteSheet;


//-------------------------------------------------------------------------------------------------
enum SpriteAnimMode
{
	SpriteAnimMode_PlayToEnd,	// Play from time=0 to durationSeconds, then finish
	SpriteAnimMode_Looping,		// Play from time=0 to end then repeat (never finish)
	//SpriteAnimMode_PingPong, 	// optional, play forwards, backwards, forwards...
	SpriteAnimMode_Count,
};


//-------------------------------------------------------------------------------------------------
class SpriteAnimation
{
private:
	const SpriteSheet*	m_spriteSheet;
	SpriteAnimMode		m_mode;
	float				m_durationSeconds;
	float				m_elapsedSeconds;
	int					m_startIndex;
	int					m_endIndex;
	int					m_currentIndex;
	bool				m_isFinished;
	bool				m_isPlaying;

public:
	SpriteAnimation( const SpriteSheet& spriteSheet, SpriteAnimMode mode, float durationSeconds, int startSpriteIndex, int endSpriteIndex );
	~SpriteAnimation( );

	void Update( float deltaSeconds );
	AABB2f GetCurrentTexCoords( ) const;	// Based on the current elapsed time
	const Texture& GetTexture( ) const;
	void Pause( ); // Starts unpaused (playing) by default
	void Resume( );	// Resume after pausing
	void Reset( ); // Rewinds to time 0 and starts (re)playing
	bool IsFinished( ) const { return m_isFinished; }
	bool IsPlaying( ) const { return m_isPlaying; }
	float GetDurationSeconds( ) const { return m_durationSeconds; }
	float GetSecondsElapsed( ) const { return m_elapsedSeconds; }
	float GetSecondsRemaining( ) const;
	float GetFractionElapsed( ) const;
	float GetFractionRemaining( ) const;
	void SetSecondsElapsed( float secondsElapsed ); // Jump to specific time
	void SetFractionElapsed( float fractionElapsed ); // e.g. 0.33f for one-third in
};
*/