#pragma once

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class RenderState
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const RenderState BASIC_3D;
	static const RenderState BASIC_2D;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	bool m_backfaceCullingEnabled;
	bool m_depthTestingEnabled;
	bool m_depthWritingEnabled;
	eBlending m_blendingMode;
	eDrawMode m_drawMode;
	bool m_windingClockwise;
	float m_lineWidth;
	//GrpahicsDevice,
	//IsDisposed,
	//AntiAlias,
	//Name,
	//ScissorTestEnable,
	//SlopeScaleDepthBias,
	//Tag,

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	RenderState( bool backfaceCulling, bool depthTesting, bool depthWriting, eBlending const & blendingMode, eDrawMode const & drawMode, bool windingClockwise );
	~RenderState( );
};