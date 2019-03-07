#include "Engine/RenderSystem/Light.hpp"

#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
//Point Light
Light::Light( Vector3f const & position, Color const & color, float radius, float falloff, bool global /*= false */ )
	: m_position( position )
	, m_lightDirection( Vector3f::ONE )
	, m_lightColor( color )
	, m_isLightDirectional( 0.f )

	, m_minLightDistance( radius )
	, m_maxLightDistance( radius + falloff )
	, m_strengthAtMin( 1.f )
	, m_strengthAtMax( 0.f )

	, m_innerLightCosAngle( 1.f )
	, m_outerLightCosAngle( -1.f )
	, m_strengthInside( 1.f )
	, m_strengthOutside( 1.f )
{
	if ( global )
	{
		m_strengthAtMax = 1.f;
	}
}


//-------------------------------------------------------------------------------------------------
//Directional Light
Light::Light( Vector3f const & position, Color const & color, Vector3f const &lightDirection, float radius, float falloff, bool global /*= false */ )
	: m_position( position )
	, m_lightDirection( lightDirection )
	, m_lightColor( color )
	, m_isLightDirectional( 1.f )

	, m_minLightDistance( radius )
	, m_maxLightDistance( radius + falloff )
	, m_strengthAtMin( 1.f )
	, m_strengthAtMax( 0.f )

	, m_innerLightCosAngle( 1.f )
	, m_outerLightCosAngle( -1.f )
	, m_strengthInside( 1.f )
	, m_strengthOutside( 1.f )
{
	if ( global )
	{
		m_strengthAtMax = 1.f;
	}
}


//-------------------------------------------------------------------------------------------------
//Spot Light
Light::Light( Vector3f const & position, Color const & color, Vector3f const & lightDirection, float radius, float falloff, float innerDegrees, float outerDegrees, bool global /*= false */ )
	: m_position( position )
	, m_lightDirection( lightDirection )
	, m_lightColor( color )
	, m_isLightDirectional( 0.f )

	, m_minLightDistance( radius )
	, m_maxLightDistance( radius + falloff )
	, m_strengthAtMin( 1.f )
	, m_strengthAtMax( 0.f )

	, m_innerLightCosAngle( CosDegrees( innerDegrees ) )
	, m_outerLightCosAngle( CosDegrees( innerDegrees + outerDegrees) )
	, m_strengthInside( 1.f )
	, m_strengthOutside( 0.f )
{
	if ( global )
	{
		m_strengthAtMax = 1.f;
	}
}