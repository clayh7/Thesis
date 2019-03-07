#include "Engine/RenderSystem/Skeleton.hpp"

#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
Skeleton::~Skeleton( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void Skeleton::AddJoint( const char * jointName, int parentJointIndex, Matrix4f const &initialBoneToModelMatrix )
{
	m_names.push_back( std::string( jointName ) );
	m_parentIndicies.push_back( parentJointIndex );
	m_boneToModeSpace.push_back( initialBoneToModelMatrix );
	Matrix4f inverse = initialBoneToModelMatrix;
	inverse.InvertOrthonormal( );
	Matrix4f modelToBoneMatrix = inverse;
	m_modelToBoneSpace.push_back( modelToBoneMatrix );
}


//-------------------------------------------------------------------------------------------------
void Skeleton::ReadFromFile( std::string const & filename )
{
	FileBinaryReader reader;
	if( reader.Open( filename ) )
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
void Skeleton::WriteToFile( std::string const & filename ) const
{
	FileBinaryWriter writer;
	if( writer.Open( filename ) )
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
std::string const & Skeleton::GetName( int index ) const
{
	return m_names[index];
}


//-------------------------------------------------------------------------------------------------
Vector3f Skeleton::GetJointPosition( int index ) const
{
	return m_boneToModeSpace[index].GetRow( 3 ).XYZ();
}


//-------------------------------------------------------------------------------------------------
bool Skeleton::GetBonePositions( int index, Vector3f * out_position0, Vector3f * out_position1 ) const
{
	int parentIndex = m_parentIndicies[index];
	if ( parentIndex == -1 )
	{
		out_position0 = nullptr;
		out_position1 = nullptr;
		return false;
	}

	*out_position0 = m_boneToModeSpace[index].GetRow( 3 ).XYZ( );
	*out_position1 = m_boneToModeSpace[parentIndex].GetRow( 3 ).XYZ( );
	return true;
}


//-------------------------------------------------------------------------------------------------
int Skeleton::GetJointCount( ) const
{
	return ( int ) m_parentIndicies.size( );
}


//-------------------------------------------------------------------------------------------------
int Skeleton::GetLastAddJointIndex( ) const
{
	return m_parentIndicies.size( ) - 1;
}


//-------------------------------------------------------------------------------------------------
int Skeleton::GetJointIndexFromName( std::string const & name ) const
{
	for ( int jointIndex = 0; jointIndex < GetJointCount( ); ++jointIndex )
	{
		if ( m_names[jointIndex] == name )
		{
			return jointIndex;
		}
	}

	return -1;
}


//-------------------------------------------------------------------------------------------------
Joint Skeleton::GetJoint( int const index ) const
{
	Joint jointData;

	jointData.name = m_names[index];
	jointData.parentIndex = m_parentIndicies[index];
	jointData.modelToBoneSpace = m_modelToBoneSpace[index];
	jointData.boneToModelSpace = m_boneToModeSpace[index];

	return jointData;
}


//-------------------------------------------------------------------------------------------------
std::vector<Matrix4f> Skeleton::GetCurrentMatricies( ) const
{
	std::vector<Matrix4f> result;
	for( unsigned int jointIndex = 0; jointIndex < m_modelToBoneSpace.size(); ++jointIndex )
	{
		result.push_back( m_modelToBoneSpace[jointIndex] * m_boneToModeSpace[jointIndex] );
	}
	return result;
}


//-------------------------------------------------------------------------------------------------
void Skeleton::SetJointWorldTransform( int jointIndex, Matrix4f const & worldTransform )
{
	m_boneToModeSpace[jointIndex] = worldTransform;
}


//-------------------------------------------------------------------------------------------------
void Skeleton::ReadFromStream( IBinaryReader &reader )
{
	//FILE VERSION
	uint32_t version;
	reader.Read<uint32_t>( &version );
	if ( version != FILE_VERSION )
	{
		ERROR_AND_DIE( "Wrong file version! Update your file by loading the fbx and saving the skeleton again." );
	}

	m_names.clear( );
	m_parentIndicies.clear( );
	m_modelToBoneSpace.clear( );
	m_boneToModeSpace.clear( );

	//Number of joints in skeleton
	uint32_t jointCount;
	reader.Read<uint32_t>( &jointCount );

	m_names.resize( jointCount );
	m_parentIndicies.resize( jointCount );
	m_modelToBoneSpace.resize( jointCount );
	m_boneToModeSpace.resize( jointCount );

	//Read all joint data
	float matData[16];
	for ( int jointIndex = 0; jointIndex < (int)jointCount; ++jointIndex )
	{
		reader.ReadString( &m_names[jointIndex] );
		reader.Read<int>( &m_parentIndicies[jointIndex] );
		for ( int matIndex = 0; matIndex < 16; ++matIndex )
		{
			reader.Read<float>( &matData[matIndex] );
		}
		m_modelToBoneSpace[jointIndex] = matData;
		for ( int matIndex = 0; matIndex < 16; ++matIndex )
		{
			reader.Read<float>( &matData[matIndex] );
		}
		m_boneToModeSpace[jointIndex] = matData;
	}
}


//-------------------------------------------------------------------------------------------------
void Skeleton::WriteToStream( IBinaryWriter &writer ) const
{
	//FILE VERSION
	writer.Write<uint32_t>( FILE_VERSION );

	//Number of joints in skeleton
	int jointCount = GetJointCount( );
	writer.Write<uint32_t>( jointCount );

	//Write all joint data
	for ( int jointIndex = 0; jointIndex < jointCount; ++jointIndex )
	{
		writer.WriteString( m_names[jointIndex] );
		writer.Write<int>( m_parentIndicies[jointIndex] );
		for ( int matIndex = 0; matIndex < 16; ++matIndex )
		{
			writer.Write<float>( m_modelToBoneSpace[jointIndex].m_data[matIndex] );
		}
		for ( int matIndex = 0; matIndex < 16; ++matIndex )
		{
			writer.Write<float>( m_boneToModeSpace[jointIndex].m_data[matIndex] );
		}
	}
}