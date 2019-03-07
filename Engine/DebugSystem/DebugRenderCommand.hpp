#pragma once

#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/Color.hpp"


//-------------------------------------------------------------------------------------------------
enum CommandType
{
	CommandType_POINT,
	CommandType_LINE,
	CommandType_ARROW,
	CommandType_AABB3,
	CommandType_SPHERE,
	//CommandType_BASIS,
	//CommandType_ROTATION,
	CommandType_GRID,
	CommandType_QUAD,
	CommandType_COUNT,
};


//-------------------------------------------------------------------------------------------------
enum DepthMode
{
	DepthMode_ON,
	DepthMode_OFF,
	DepthMode_DUAL,
};


//-------------------------------------------------------------------------------------------------
class DebugRenderCommand
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	CommandType m_type;
	Vector3f m_startPosition;
	Vector3f m_endPosition;
	Vector3f m_rightVector;
	Vector3f m_upVector;
	Color m_startColor;
	Color m_endColor;
	Color m_startOutlineColor;
	Color m_endOutlineColor;
	AABB2f m_texCoords;
	float m_radius;
	float m_size;
	float m_duration;
	float m_age;
	DepthMode m_mode;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	DebugRenderCommand( CommandType const & type, Vector3f const & startPosition, Vector3f const & endPosition, Color const & startColor, Color const & endColor, Color const & startOutlineColor, Color const & endOutlineColor, float radius, float size, float duration, DepthMode const & mode );
	DebugRenderCommand( CommandType const & type, Vector3f const & startPosition, Vector3f const & endPosition, Color const & startColor, Color const & endColor, float size, float duration, DepthMode const & mode );
	DebugRenderCommand( CommandType const & type, Vector3f const & position, Vector3f const & rightVector, Vector3f const & upVector, Color const & startColor, Color const & endColor, AABB2f const & texCoords, float length, float lineSize, float duration, DepthMode const & mode );
	bool IsComplete( ) const;
};