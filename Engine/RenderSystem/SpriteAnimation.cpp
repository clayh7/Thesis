#include "Engine/RenderSystem/SpriteAnimation.hpp"

/*
#include "Engine/RenderSystem/SpriteSheet.hpp"


//-------------------------------------------------------------------------------------------------
SpriteAnimation::SpriteAnimation( const SpriteSheet& spriteSheet, SpriteAnimMode mode, float durationSeconds, int startSpriteIndex, int endSpriteIndex )
: m_spriteSheet( &spriteSheet )
, m_mode( mode )
, m_durationSeconds( durationSeconds )
, m_elapsedSeconds( 0.f )
, m_startIndex( startSpriteIndex )
, m_endIndex( endSpriteIndex )
, m_currentIndex( startSpriteIndex )
, m_isFinished( false )
, m_isPlaying( true )
{
}


//-------------------------------------------------------------------------------------------------
SpriteAnimation::~SpriteAnimation( )
{

}


//-------------------------------------------------------------------------------------------------
void SpriteAnimation::Update( float deltaSeconds )
{
	if ( m_isPlaying )
		m_elapsedSeconds += deltaSeconds;

	float indexTotalRange = m_endIndex - m_startIndex + 1.f;
	float fractionElapsed = GetFractionElapsed( );

	if ( m_mode == SpriteAnimMode_PlayToEnd)
	{
		if ( fractionElapsed > 1.f )
		{
			fractionElapsed = 1.f;
			m_isFinished = true;
		}
	}
	else if ( m_mode == SpriteAnimMode_Looping )
	{
		while ( fractionElapsed > 1.f )
		{
			fractionElapsed -= 1.f;
		}
	}

	float currentIndexFloat = indexTotalRange*fractionElapsed;
	m_currentIndex = ( int )floor( currentIndexFloat ) + m_startIndex;
}


//-------------------------------------------------------------------------------------------------
AABB2f SpriteAnimation::GetCurrentTexCoords( ) const
{
	return m_spriteSheet->GetTexCoordsForSpriteIndex( m_currentIndex );
}


//-------------------------------------------------------------------------------------------------
const Texture& SpriteAnimation::GetTexture( ) const
{
	return m_spriteSheet->GetTexture();
}


//-------------------------------------------------------------------------------------------------
void SpriteAnimation::Pause( )
{
	m_isPlaying = false;
}


//-------------------------------------------------------------------------------------------------
void SpriteAnimation::Resume( )
{
	m_isPlaying = true;
}


//-------------------------------------------------------------------------------------------------
void SpriteAnimation::Reset( )
{
	m_currentIndex = 0;
	m_isPlaying = true;
	m_isFinished = false;
}


//-------------------------------------------------------------------------------------------------
float SpriteAnimation::GetSecondsRemaining( ) const
{
	return m_durationSeconds - m_elapsedSeconds;
}


//-------------------------------------------------------------------------------------------------
float SpriteAnimation::GetFractionElapsed( ) const
{
	return m_elapsedSeconds / m_durationSeconds;
}


//-------------------------------------------------------------------------------------------------
float SpriteAnimation::GetFractionRemaining( ) const
{
	return 1 - (m_elapsedSeconds / m_durationSeconds);
}


//-------------------------------------------------------------------------------------------------
void SpriteAnimation::SetSecondsElapsed( float secondsElapsed )
{
	m_elapsedSeconds = secondsElapsed;
	Update( 0.f );
}


//-------------------------------------------------------------------------------------------------
void SpriteAnimation::SetFractionElapsed( float fractionElapsed )
{
	m_elapsedSeconds = m_durationSeconds * fractionElapsed;
	Update( 0.f );
}
*/