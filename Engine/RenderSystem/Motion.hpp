#pragma once

#include <string>
#include <vector>
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/RenderSystem/Skeleton.hpp"


//-------------------------------------------------------------------------------------------------
enum eExtrapolationMode
{
	eExtrapolationMode_CLAMP,
	eExtrapolationMode_LOOP,
	eExtrapolationMode_PING_PONG,
};


//-------------------------------------------------------------------------------------------------
class Motion
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	// 1: Initial Version
	static uint32_t const FILE_VERSION = 1;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	std::string m_name;
	eExtrapolationMode m_extrapolationMode;
	float m_totalLengthSeconds;
	float m_frameRate;
	float m_frameTime;
	int m_jointCount;
	int m_frameCount;
	std::vector<Matrix4f> m_keyframes;

	Skeleton * m_targetSkeleton;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Motion( std::string const & motionName, float timeSpan, float framerate, Skeleton * skeleton );
	~Motion( );

	void ApplyMotionToSkeleton( float time );

	int GetFrameCount( );
	void GetFrameIndicesWithBlend( float inTime, uint32_t * out_frameIndex0, uint32_t * out_frameIndex1, float * out_blend );
	Matrix4f GetJointKeyframe( uint32_t jointIndex, uint32_t keyframe );
	uint32_t GetJointKeyframeIndex( uint32_t jointIndex, uint32_t keyframe );

	void SetTransform( uint32_t jointIndex, uint32_t frameIndex, Matrix4f const & boneTransform );
	void SetExtrapolationMode( eExtrapolationMode const & mode );
	void SetTargetSkeleton( Skeleton * targetSkeleton );
	void ReadFromFile( std::string const & filename );
	void WriteToFile( std::string const & filename );
	void ReadFromStream( IBinaryReader & reader );
	void WriteToStream( IBinaryWriter & writer ) const;
};