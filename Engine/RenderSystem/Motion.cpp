#include "Engine/RenderSystem/Motion.hpp"

#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
Motion::Motion( std::string const & motionName, float timeSpan, float framerate, Skeleton * skeleton )
	: m_name( motionName )
	, m_extrapolationMode( eExtrapolationMode_CLAMP )
	, m_totalLengthSeconds( timeSpan )
	, m_frameRate( framerate )
	, m_frameTime( 0.f )
	, m_jointCount( 0 )
	, m_frameCount( 0 )
	, m_targetSkeleton( skeleton )
{
	m_frameCount = ( int ) ceil( framerate * timeSpan ) + 1;
	if ( m_targetSkeleton == nullptr )
	{
		m_jointCount = 0;
	}
	else
	{
		m_jointCount = m_targetSkeleton->GetJointCount( );
	}
	m_frameTime = 1.0f / framerate;
	m_keyframes.resize( m_frameCount * m_jointCount );
}


//-------------------------------------------------------------------------------------------------
Motion::~Motion( )
{
}


//-------------------------------------------------------------------------------------------------
void Motion::ApplyMotionToSkeleton( float time )
{
	//Temporary Looping time
	switch ( m_extrapolationMode )
	{
	case eExtrapolationMode_CLAMP:
		time = Clamp( time, 0.f, m_totalLengthSeconds );
		break;
	case eExtrapolationMode_LOOP:
		time = fmodf( time, m_totalLengthSeconds );
		break;
	case eExtrapolationMode_PING_PONG:
		time = fmodf( time, m_totalLengthSeconds * 2.f );
		if ( time > m_totalLengthSeconds )
		{
			time = ( m_totalLengthSeconds * 2.f ) - time;
		}
		break;
	}

	uint32_t frame0;
	uint32_t frame1;
	float blend;
	GetFrameIndicesWithBlend( time, &frame0, &frame1, &blend );

	uint32_t jointCount = m_targetSkeleton->GetJointCount( );
	for ( uint32_t jointIndex = 0; jointIndex < jointCount; ++jointIndex )
	{
		Matrix4f mat0 = GetJointKeyframe( jointIndex, frame0 );
		Matrix4f mat1 = GetJointKeyframe( jointIndex, frame1 );

		Matrix4f newModel = LerpMatrix( mat0, mat1, blend );

		// Needs to set bone_to_model
		// (or set your matrix tree's world to this, and set
		// bone to model on Skeleton world's array)
		m_targetSkeleton->SetJointWorldTransform( jointIndex, newModel );
	}
}


//-------------------------------------------------------------------------------------------------
int Motion::GetFrameCount( )
{
	return m_frameCount;
}


//-------------------------------------------------------------------------------------------------
void Motion::GetFrameIndicesWithBlend( float inTime, uint32_t * out_frameIndex0, uint32_t * out_frameIndex1, float * out_blend )
{
	*out_frameIndex0 = ( uint32_t ) floor( inTime / m_frameTime );
	*out_frameIndex1 = *out_frameIndex0 + 1;

	if ( *out_frameIndex0 == ( uint32_t ) ( m_frameCount - 1 ) )
	{
		*out_frameIndex1 = m_frameCount - 1;
		*out_blend = 0.0f;
	}
	else if ( *out_frameIndex0 == ( uint32_t ) ( m_frameCount - 2 ) )
	{
		float lastFrameTime = m_totalLengthSeconds - ( m_frameTime * *out_frameIndex0 );
		*out_blend = fmodf( inTime, m_frameTime ) / lastFrameTime;
		*out_blend = Clamp( *out_blend, 0.f, 1.f );
	}
	else
	{
		*out_blend = fmodf( inTime, m_frameTime ) / m_frameTime;
	}
}


//-------------------------------------------------------------------------------------------------
Matrix4f Motion::GetJointKeyframe( uint32_t jointIndex, uint32_t keyframe )
{
	ASSERT_RECOVERABLE(  jointIndex < ( uint32_t ) m_jointCount, "Joint Index out of bounds" );
	return m_keyframes[m_frameCount * jointIndex + keyframe];
}


//-------------------------------------------------------------------------------------------------
uint32_t Motion::GetJointKeyframeIndex( uint32_t jointIndex, uint32_t keyframe )
{
	return m_frameCount * jointIndex + keyframe;
}


//-------------------------------------------------------------------------------------------------
void Motion::SetTransform( uint32_t jointIndex, uint32_t frameIndex, Matrix4f const & boneTransform )
{
	uint32_t boneKeyframeIndex = GetJointKeyframeIndex( jointIndex, frameIndex );
	m_keyframes[boneKeyframeIndex] = boneTransform;
}


//-------------------------------------------------------------------------------------------------
void Motion::SetExtrapolationMode( eExtrapolationMode const & mode )
{
	m_extrapolationMode = mode;
}


//-------------------------------------------------------------------------------------------------
void Motion::SetTargetSkeleton( Skeleton * targetSkeleton )
{
	m_targetSkeleton = targetSkeleton;
}


//-------------------------------------------------------------------------------------------------
void Motion::ReadFromFile( std::string const & filename )
{
	FileBinaryReader reader;
	if ( reader.Open( filename ) )
	{
		ReadFromStream( reader );
	}
	else
	{
		g_ConsoleSystem->AddLog( Stringf( "Cannot read: %s", filename.c_str( ) ), Color::RED );
	}
	reader.Close( );
}


//-------------------------------------------------------------------------------------------------
void Motion::WriteToFile( std::string const & filename )
{
	FileBinaryWriter writer;
	if ( writer.Open( filename ) )
	{
		WriteToStream( writer );
	}
	else
	{
		g_ConsoleSystem->AddLog( Stringf( "Cannot write to: %s", filename.c_str( ) ), Color::RED );
	}
	writer.Close( );
}


//-------------------------------------------------------------------------------------------------
void Motion::ReadFromStream( IBinaryReader &reader )
{
	//FILE VERSION
	uint32_t version;
	reader.Read<uint32_t>( &version );
	if ( version != FILE_VERSION )
	{
		ERROR_AND_DIE( "Wrong file version! Update your file by loading the fbx and saving the skeleton again." );
	}

	//Read Meta data
	reader.ReadString( &m_name );
	int mode;
	reader.Read<int>( &mode );
	m_extrapolationMode = ( eExtrapolationMode ) mode;
	reader.Read<float>( &m_totalLengthSeconds );
	reader.Read<float>( &m_frameRate );
	reader.Read<float>( &m_frameTime );
	reader.Read<int>( &m_jointCount );
	reader.Read<int>( &m_frameCount );

	m_keyframes.clear( );

	//Number of joints in skeleton
	uint32_t keyframeCount;
	reader.Read<uint32_t>( &keyframeCount );

	m_keyframes.resize( keyframeCount );

	//Read all keyframe data
	float matData[16];
	uint32_t keyframeIndex = 0;

	while ( keyframeIndex < keyframeCount )
	{
		uint32_t keyframeNum;
		reader.Read<uint32_t>( &keyframeNum ); //#TODO: randomly becomes large for UnityChan
		for ( int matIndex = 0; matIndex < 16; ++matIndex )
		{
			reader.Read<float>( &matData[matIndex] );
		}
		for ( uint32_t repeatMat = 0; repeatMat < keyframeNum; ++repeatMat )
		{
			m_keyframes[keyframeIndex] = matData;
			keyframeIndex++;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Motion::WriteToStream( IBinaryWriter &writer ) const
{
	//FILE VERSION
	writer.Write<uint32_t>( FILE_VERSION );

	//Write Meta data
	writer.WriteString( m_name );
	writer.Write<int>( m_extrapolationMode );
	writer.Write<float>( m_totalLengthSeconds );
	writer.Write<float>( m_frameRate );
	writer.Write<float>( m_frameTime );
	writer.Write<int>( m_jointCount );
	writer.Write<int>( m_frameCount );

	//Write all keyframes
	uint32_t keyframeCount = m_keyframes.size( );
	writer.Write<uint32_t>( keyframeCount );

	//Write all keyframe data (RLE)
	uint32_t keyframeNum = 1;
	Matrix4f currentKeyframeMatrix = m_keyframes[0];
	for ( uint32_t keyframeIndex = 1; keyframeIndex < keyframeCount; ++keyframeIndex )
	{
		if ( m_keyframes[keyframeIndex] == currentKeyframeMatrix )
		{
			keyframeNum++;
		}
		else
		{
			writer.Write<uint32_t>( keyframeNum );
			for ( int matIndex = 0; matIndex < 16; ++matIndex )
			{
				writer.Write<float>( currentKeyframeMatrix.m_data[matIndex] );
			}
			keyframeNum = 1;
			currentKeyframeMatrix = m_keyframes[keyframeIndex];
		}
	}

	//Write last keyframe data
	writer.Write<uint32_t>( keyframeNum );
	for ( int matIndex = 0; matIndex < 16; ++matIndex )
	{
		writer.Write<float>( currentKeyframeMatrix.m_data[matIndex] );
	}
}