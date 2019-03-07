#pragma once

#include "Engine/Math/Vector3f.hpp"
#include "Engine/RenderSystem/Color.hpp"


//-------------------------------------------------------------------------------------------------
class Light
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Vector3f m_lightDirection;
	Color m_lightColor;
	//1.f = true | 0.f = false
	float m_isLightDirectional;

	float m_minLightDistance;
	float m_maxLightDistance;
	float m_strengthAtMin;
	float m_strengthAtMax;

	float m_innerLightCosAngle;
	float m_outerLightCosAngle;
	float m_strengthInside;
	float m_strengthOutside;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	//Point Light
	Light( Vector3f const & position, Color const & color, float radius, float falloff, bool global = false );

	//Directional Light
	Light( Vector3f const & position, Color const & color, Vector3f const & lightDirection, float radius, float falloff, bool global = false );

	//Spot Light
	Light( Vector3f const & position, Color const & color, Vector3f const & lightDirection, float radius, float falloff, float innerDegrees, float outerDegrees, bool global = false );
};