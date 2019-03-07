#include "Engine/DebugSystem/DebugRenderCommand.hpp"


//-------------------------------------------------------------------------------------------------
DebugRenderCommand::DebugRenderCommand( const CommandType& type, const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, const Color& startOutlineColor, const Color& endOutlineColor, float radius, float size, float duration, const DepthMode& mode )
	: m_type( type )
	, m_startPosition( startPosition )
	, m_endPosition( endPosition )
	, m_rightVector( Vector3f::ZERO )
	, m_upVector( Vector3f::ZERO )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_startOutlineColor( startOutlineColor )
	, m_endOutlineColor( endOutlineColor )
	, m_texCoords( AABB2f::ZERO_TO_ZERO )
	, m_radius( radius )
	, m_size( size )
	, m_duration( duration )
	, m_age( 0.f )
	, m_mode( mode )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
DebugRenderCommand::DebugRenderCommand( const CommandType& type, const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, float size, float duration, const DepthMode& mode )
	: m_type( type )
	, m_startPosition( startPosition )
	, m_endPosition( endPosition )
	, m_rightVector( Vector3f::ZERO )
	, m_upVector( Vector3f::ZERO )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_startOutlineColor( startColor )
	, m_endOutlineColor( endColor )
	, m_texCoords( AABB2f::ZERO_TO_ZERO )
	, m_radius( 0 )
	, m_size( size )
	, m_duration( duration )
	, m_age( 0.f )
	, m_mode( mode )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
DebugRenderCommand::DebugRenderCommand( const CommandType& type, const Vector3f& position, const Vector3f& rightVector, const Vector3f& upVector, const Color& startColor, const Color& endColor, const AABB2f& texCoords, float length, float lineSize, float duration, const DepthMode& mode )
	: m_type( type )
	, m_startPosition( position )
	, m_endPosition( position )
	, m_rightVector( rightVector )
	, m_upVector( upVector )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_startOutlineColor( startColor )
	, m_endOutlineColor( endColor )
	, m_texCoords( texCoords )
	, m_radius( length )
	, m_size( lineSize )
	, m_duration( duration )
	, m_age( 0.f )
	, m_mode( mode )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
bool DebugRenderCommand::IsComplete( ) const
{
	return m_age >= m_duration;
}