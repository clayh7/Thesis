#include "Engine/Math/Euler.hpp"
#define STATIC


//-------------------------------------------------------------------------------------------------
STATIC const Euler Euler::ZERO( 0.f, 0.f, 0.f );


//-------------------------------------------------------------------------------------------------
Euler::Euler( float pitchX, float yawY, float rollZ )
: m_pitchDegreesAboutX( pitchX )
, m_yawDegreesAboutY( yawY )
, m_rollDegreesAboutZ( rollZ )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Euler::~Euler( )
{
	//Nothing
}