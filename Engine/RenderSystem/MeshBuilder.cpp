#include "Engine/RenderSystem/MeshBuilder.hpp"

#include <stddef.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/Skeleton.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
//#TODO: Make this work with/without IBOs and with mixed ibo and non-ibo draw instructions
STATIC std::vector<MeshBuilder*> MeshBuilder::CombineAllWithSameMaterialID( std::vector<MeshBuilder*> const & meshBuilders )
{
	std::vector<MeshBuilder*> combinedMeshBuilders;
	for ( MeshBuilder const * mb : meshBuilders )
	{
		MeshBuilder * foundMb = nullptr;

		//Check if materialID already exists
		for ( MeshBuilder * checkMb : combinedMeshBuilders )
		{
			if ( checkMb->GetMaterialID( ) == mb->GetMaterialID( ) )
			{
				foundMb = checkMb;
			}
		}

		//If not, make a new MeshBuilder
		if ( foundMb == nullptr )
		{
			foundMb = new MeshBuilder( );
			combinedMeshBuilders.push_back( foundMb );
		}

		//Check is using IBOs
		foundMb->m_useIBO = mb->m_useIBO;
		
		//Add all verts
		unsigned int currentIndex = ( unsigned int ) foundMb->m_vertexes.size( );
		for ( Vertex_Master const & addVert : mb->GetVertexData( ) )
		{
			foundMb->m_vertexes.push_back( addVert );
		}

		//Add all indicies
		for ( unsigned int const & addIndex : mb->GetIndexBuffer( ) )
		{
			foundMb->m_indicies.push_back( currentIndex + addIndex );
		}

		for ( DrawInstruction di : mb->GetDrawInstructions( ) )
		{
			di.m_useIndexBuffer = foundMb->m_useIBO;
			di.m_startIndex = foundMb->m_startVertex;
			foundMb->m_drawInstructions.push_back( di );
			foundMb->m_startVertex += di.m_count;
		}
		foundMb->m_materialID = mb->m_materialID;
		foundMb->m_dataMask |= mb->GetDataMask( );
	}

	return combinedMeshBuilders;
}


//-------------------------------------------------------------------------------------------------
STATIC MeshBuilder * MeshBuilder::CreateSkeletonMesh( Skeleton const * skeleton )
{
	MeshBuilder * builder = new MeshBuilder( );

	Vector3f start;
	Vector3f end;
	builder->SetColor( Color::BLACK );
	for ( int boneIndex = 0; boneIndex < skeleton->GetJointCount( ); ++boneIndex )
	{
		if ( skeleton->GetBonePositions( boneIndex, &start, &end ) )
		{
			builder->AddLine( start, end );
		}
	}

	builder->SetColor( Color::RED );
	for ( int jointIndex = 0; jointIndex < skeleton->GetJointCount( ); ++jointIndex )
	{
		builder->AddCube( skeleton->GetJointPosition( jointIndex ), 0.5f );
		builder->SetColor( Color::WHITE );
	}

	return builder;
}


//-------------------------------------------------------------------------------------------------
MeshBuilder::MeshBuilder( bool useIBO /*= false */ )
	: m_useIBO( useIBO )
	, m_startVertex( 0 )
	, m_dataMask( 0 )
	, m_materialID( "" )
	, m_vertexStamp( )
	, m_instructionStamp( useIBO )
{
}


//-------------------------------------------------------------------------------------------------
MeshBuilder::~MeshBuilder( )
{

}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::Begin( )
{
	if ( m_useIBO )
	{
		m_startVertex = m_indicies.size( );
	}
	else
	{
		m_startVertex = m_vertexes.size( );
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::End( )
{
	int vertexCount = 0;
	if ( m_useIBO )
	{
		vertexCount = m_indicies.size( );
	}
	else
	{
		vertexCount = m_vertexes.size( );
	}
	m_instructionStamp.m_startIndex = m_startVertex;
	m_instructionStamp.m_count = vertexCount - m_startVertex;

	m_drawInstructions.push_back( m_instructionStamp );
	m_startVertex = vertexCount;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetPrimitiveType( ePrimitiveType const & pType )
{
	m_instructionStamp.SetPrimitiveType( pType );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetColor( Color const &color )
{
	m_vertexStamp.m_color = color;
	m_dataMask |= BIT( eMeshData_COLOR );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetTangent( Vector3f const &tangent )
{
	m_vertexStamp.m_tangent = tangent;
	m_dataMask |= BIT( eMeshData_TANGENT );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetBitangent( Vector3f const &bitangent )
{
	m_vertexStamp.m_bitangent = bitangent;
	m_dataMask |= BIT( eMeshData_BITANGENT );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetNormal( Vector3f const &normal )
{
	m_vertexStamp.m_normal = normal;
	m_dataMask |= BIT( eMeshData_NORMAL );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetTBN( Vector3f const &tangent, Vector3f const &bitangent, Vector3f const &normal )
{
	SetTangent( tangent );
	SetBitangent( bitangent );
	SetNormal( normal );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetUV0( Vector2f const &uv )
{
	m_vertexStamp.m_uv0 = uv;
	m_dataMask |= BIT( eMeshData_UV0 );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetUV0( float u, float v )
{
	SetUV0( Vector2f( u, v ) );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetUV1( Vector2f const &uv )
{
	m_vertexStamp.m_uv1 = uv;
	m_dataMask |= BIT( eMeshData_UV1 );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetUV1( float u, float v )
{
	SetUV1( Vector2f( u, v ) );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::SetBoneWeights( Vector4i const & boneIndicies, Vector4f const & boneWeights )
{
	m_vertexStamp.m_boneIndicies = boneIndicies;
	m_vertexStamp.m_boneWeights = boneWeights;
	m_dataMask |= BIT( eMeshData_BONEWEIGHTS );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddVertex( Vector3f const &position )
{
	m_vertexStamp.m_position = position;
	m_vertexes.push_back( m_vertexStamp );
	m_dataMask |= BIT( eMeshData_POSITION );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddTriangle( Vector3f const & topLeft, Vector3f const & bottomLeft, Vector3f const & bottomRight )
{
	Begin( );

	SetColor( Color::WHITE );

	// Let's calculate some TBN information
	Vector3f tangent = bottomRight - bottomLeft;
	Vector3f bitangent = topLeft - bottomLeft;
	tangent.Normalize( );
	bitangent.Normalize( );
	Vector3f normal = CrossProduct( tangent, bitangent );
	bitangent = CrossProduct( normal, tangent );

	SetTBN( tangent, bitangent, normal );

	SetUV0( 0.0f, 0.0f );
	AddVertex( bottomLeft );

	SetUV0( 1.0f, 0.0f );
	AddVertex( bottomRight );

	SetUV0( 0.0f, 1.0f );
	AddVertex( topLeft );

	uint32_t startIndex = GetNextIndex( );
	AddIndex( startIndex + 0 );
	AddIndex( startIndex + 1 );
	AddIndex( startIndex + 2 );

	End( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddQuad( Vector3f const & topLeft, Vector3f const & bottomRight )
{
	AddQuad( topLeft, bottomRight, AABB2f::ZERO_TO_ONE );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddQuad( Vector3f const & topLeft, Vector3f const & bottomRight, AABB2f const & uvBounds )
{
	Vector3f bottomLeft = Vector3f( topLeft.x, bottomRight.y, 0.f );
	Vector3f topRight = Vector3f( bottomRight.x, topLeft.y, 0.f );
	Vector2f uvTopLeft = Vector2f( uvBounds.mins.x, uvBounds.maxs.y );
	Vector2f uvTopRight = Vector2f( uvBounds.maxs.x, uvBounds.maxs.y );
	Vector2f uvBottomLeft = Vector2f( uvBounds.mins.x, uvBounds.mins.y );
	Vector2f uvBottomRight = Vector2f( uvBounds.maxs.x, uvBounds.mins.y );

	AddQuad( topLeft, bottomLeft, bottomRight, topRight, uvTopLeft, uvBottomLeft, uvBottomRight, uvTopRight );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddQuad( Vector3f const & topLeft, Vector3f const & bottomLeft, Vector3f const & bottomRight, Vector3f const & topRight )
{
	AddQuad( topLeft, bottomLeft, bottomRight, topRight, Vector2f( 0.f, 1.f ), Vector2f( 0.f, 0.f ), Vector2f( 1.f, 0.f ), Vector2f( 1.f, 1.f ) );
}

//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddQuad( Vector3f const & topLeft, Vector3f const & bottomLeft, Vector3f const & bottomRight, Vector3f const & topRight,
	Vector2f const & uvTopLeft, Vector2f const & uvBottomLeft, Vector2f const & uvBottomRight, Vector2f const & uvTopRight )
{
	Begin( );

	SetColor( Color::WHITE );

	// Let's calculate some TBN information
	Vector3f tangent = bottomRight - bottomLeft;
	Vector3f bitangent = topLeft - bottomLeft;
	tangent.Normalize( );
	bitangent.Normalize( );
	Vector3f normal = CrossProduct( tangent, bitangent );
	bitangent = CrossProduct( normal, tangent );

	SetTBN( tangent, bitangent, normal );

	SetUV0( uvTopLeft ); //TL 0,1
	AddVertex( topLeft );

	SetUV0( uvTopRight ); //TR 1,1
	AddVertex( topRight );

	SetUV0( uvBottomLeft ); //BL 0,0
	AddVertex( bottomLeft );

	SetUV0( uvBottomRight ); //BR 1,0
	AddVertex( bottomRight );

	uint32_t startIndex = GetNextIndex( );
	AddQuadIndicies( startIndex + 0, startIndex + 1, startIndex + 2, startIndex + 3 );

	End( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddPlane( Vector3f const &pos, Vector3f const &right, Vector3f const &up, float xStart, float xEnd, int xSections, float yStart, float yEnd, int ySections )
{
	ASSERT_RECOVERABLE( xSections > 0, "Not enough X Sections." );
	ASSERT_RECOVERABLE( ySections > 0, "Not enough Y Sections." );
	ASSERT_RECOVERABLE( m_useIBO, "Needs to use IBOs." );

	PlaneData plane;
	plane.initialPosition = pos;
	plane.rightVector = right;
	plane.upVector = up;

	AddPatch( &PlaneFunc, &plane, xStart, xEnd, xSections, yStart, yEnd, ySections );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddCustom( int effectIndex, float xStart, float xEnd, int xSections, float yStart, float yEnd, int ySections )
{
	ASSERT_RECOVERABLE( xSections > 0, "Not enough X Sections." );
	ASSERT_RECOVERABLE( ySections > 0, "Not enough Y Sections." );
	ASSERT_RECOVERABLE( m_useIBO, "Needs to use IBOs." );

	PlaneData plane;
	plane.initialPosition = Vector3f::ZERO;
	plane.rightVector = Vector3f( 1.f, 0.f, 0.f );
	plane.upVector = Vector3f( 0.f, 0.f, -1.f );
	plane.effectIndex = effectIndex;

	AddPatch( &WavesFunc, &plane, xStart, xEnd, xSections, yStart, yEnd, ySections );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddPatch( PatchFunc *patchFunc, void const *userData, float xStart, float xEnd, int xSections, float yStart, float yEnd, int ySections )
{
	ASSERT_RECOVERABLE( xSections > 0, "Not enough X Sections." );
	ASSERT_RECOVERABLE( ySections > 0, "Not enough Y Sections." );
	ASSERT_RECOVERABLE( m_useIBO, "Needs to use IBOs." );

	Begin( );

	// So, let's add out our vertices.
	int xVertexCount = xSections + 1;
	int yVertexCount = ySections + 1;

	float const xRange = xEnd - xStart;
	float const yRange = yEnd - yStart;
	float const xStep = xRange / ( float ) xSections;
	float const yStep = yRange / ( float ) ySections;

	// Texture goes over the whole patch
	// BONUS - you can provite uv ranges as well.
	float const uStep = 1.0f / ( float ) xSections;
	float const vStep = 1.0f / ( float ) ySections;

	int startVertexIndex = GetCurrentIndex( );

	// Okay, now, let's add all our vertices
	float x, y;
	float u, v;

	y = yStart;
	v = 0.0f;

	float const delta = .01f; // artificially small value, can go smaller

	for ( int iy = 0; iy < yVertexCount; ++iy )
	{
		x = xStart;
		u = 0.0f;

		for ( int ix = 0; ix < xVertexCount; ++ix )
		{
			SetUV0( u, v );

			// calculate tangent along u (that is, x)
			Vector3f tangent = patchFunc( userData, x + delta, y )
				- patchFunc( userData, x - delta, y );

			// calculate bitangent along v (that is, y)
			Vector3f bitangent = patchFunc( userData, x, y + delta )
				- patchFunc( userData, x, y - delta );

			tangent.Normalize( );
			bitangent.Normalize( );
			Vector3f normal = CrossProduct( tangent, bitangent );
			bitangent = CrossProduct( normal, tangent );
			SetTBN( tangent, bitangent, normal );

			Vector3f position = patchFunc( userData, x, y );
			AddVertex( position );

			x += xStep;
			u += uStep;
		}

		y += yStep;
		v += vStep;
	}

	// Next, add all the indices for this patch
	for ( int iy = 0; iy < ySections; ++iy )
	{
		for ( int ix = 0; ix < xSections; ++ix )
		{
			unsigned int bottomLeft = ( unsigned int ) ( startVertexIndex + ( iy * xVertexCount ) + ix );
			unsigned int bottomRight = ( unsigned int ) ( bottomLeft + 1 );
			unsigned int topLeft = ( unsigned int ) ( bottomLeft + xVertexCount );
			unsigned int topRight = ( unsigned int ) ( topLeft + 1 );

			// How many indices is this actually adding
			// under the hood, if we're working with triangles?
			AddQuadIndicies( topLeft, topRight, bottomLeft, bottomRight );
		}
	}

	End( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddCube( Vector3f const & pos, float size /*= 1.f */ )
{
	Begin( );

	m_instructionStamp.SetPrimitiveType( ePrimitiveType_TRIANGLES );

	float radius = size / 2.f;
	Vector3f topLeft = pos + Vector3f( -radius, radius, radius );
	Vector3f topRight = pos + Vector3f( radius, radius, radius );
	Vector3f bottomLeft = pos + Vector3f( -radius, -radius, radius );
	Vector3f bottomRight = pos + Vector3f( radius, -radius, radius );

	Vector3f topLeftBack = pos + Vector3f( radius, radius, -radius );
	Vector3f topRightBack = pos + Vector3f( -radius, radius, -radius );
	Vector3f bottomLeftBack = pos + Vector3f( radius, -radius, -radius );
	Vector3f bottomRightBack = pos + Vector3f( -radius, -radius, -radius );

	//Front
	AddVertex( bottomLeft );
	AddVertex( bottomRight );
	AddVertex( topRight );

	AddVertex( bottomLeft );
	AddVertex( topRight );
	AddVertex( topLeft );

	//Back
	AddVertex( bottomLeftBack );
	AddVertex( bottomRightBack );
	AddVertex( topRightBack );

	AddVertex( bottomLeftBack );
	AddVertex( topRightBack );
	AddVertex( topLeftBack );

	//Right
	AddVertex( bottomRight );
	AddVertex( bottomLeftBack );
	AddVertex( topLeftBack );

	AddVertex( bottomRight );
	AddVertex( topLeftBack );
	AddVertex( topRight );

	//Left
	AddVertex( bottomRightBack );
	AddVertex( bottomLeft );
	AddVertex( topLeft );

	AddVertex( bottomRightBack );
	AddVertex( topLeft );
	AddVertex( topRightBack );

	//Top
	AddVertex( topLeft );
	AddVertex( topRight );
	AddVertex( topLeftBack );

	AddVertex( topLeft );
	AddVertex( topLeftBack );
	AddVertex( topRightBack );

	//Bottom
	AddVertex( bottomRightBack );
	AddVertex( bottomLeftBack );
	AddVertex( bottomRight );

	AddVertex( bottomRightBack );
	AddVertex( bottomRight );
	AddVertex( bottomLeft );

	//AddQuadIndicies( m_startIndex + 0, m_startIndex + 1, m_startIndex + 2, m_startIndex + 3 );

	End( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddLine( Vector3f const & start, Vector3f const & end )
{
	Begin( );

	m_instructionStamp.SetPrimitiveType( ePrimitiveType_LINES );
	AddVertex( start );
	AddVertex( end );

	End( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddIndex( unsigned int index )
{
	m_indicies.push_back( index );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::AddQuadIndicies( unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br )
{
	m_indicies.push_back( bl );
	m_indicies.push_back( br );
	m_indicies.push_back( tr );

	m_indicies.push_back( bl );
	m_indicies.push_back( tr );
	m_indicies.push_back( tl );
}


//-------------------------------------------------------------------------------------------------
uint32_t MeshBuilder::GetNextIndex( ) const
{
	uint32_t largest = 0;
	for( uint32_t checkIndex : m_indicies )
	{
		if( largest <= checkIndex )
		{
			largest = checkIndex + 1;
		}
	}
	return largest;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::ClearBoneWeights( )
{
	m_vertexStamp.m_boneWeights = Vector4f( 1.f, 0.f, 0.f, 0.f );
	m_vertexStamp.m_boneIndicies = Vector4i( 0, 0, 0, 0 );
	m_dataMask &= ~BIT( eMeshData_BONEWEIGHTS );
}


//-------------------------------------------------------------------------------------------------
//#TODO: Maybe make indicies not shorts
void MeshBuilder::MeshReduction( )
{
	if ( m_useIBO )
	{
		ERROR_AND_DIE( "Currently don't support mesh reduction with IBOs" );
	}

	m_useIBO = true;

	unsigned int indexCount = 0;
	std::vector<Vertex_Master> reducedVerticies;
	std::vector<DrawInstruction> reducedInstructions;
	reducedVerticies.reserve( m_vertexes.size( ) );
	m_indicies.clear( );
	m_indicies.reserve( m_vertexes.size( ) );

	for ( DrawInstruction & instruction : m_drawInstructions )
	{
		instruction.m_useIndexBuffer = true;
		instruction.m_startIndex = reducedVerticies.size( );
		for ( unsigned int vertIndex = 0; vertIndex < instruction.m_count; ++vertIndex )
		{
			bool notFound = true;
			for ( int reducedIndex = (int)reducedVerticies.size( ) - 1; /*notFound &&*/ reducedIndex >= 0; --reducedIndex )
			{
				if ( reducedVerticies[reducedIndex].Equals( m_vertexes[vertIndex] ) )
				{
					m_indicies.push_back( ( unsigned int ) reducedIndex );
					notFound = false;
					break;
				}
			}
			if ( notFound )
			{
				reducedVerticies.push_back( m_vertexes[vertIndex] );
				m_indicies.push_back( ( unsigned int ) indexCount );
				++indexCount;
			}
		}
	}
	g_ConsoleSystem->AddLog( Stringf( "Mesh Start Verts: %d", ( int ) m_vertexes.size( ) ), Color::LIGHT_GREEN );
	g_ConsoleSystem->AddLog( Stringf( "Mesh Reduced Verts: %d", ( int ) reducedVerticies.size( ) ), Color::GREEN );
	reducedVerticies.shrink_to_fit( );
	m_vertexes = reducedVerticies;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::ReadFromFile( std::string const &filename )
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
void MeshBuilder::WriteToFile( std::string const &filename ) const
{
	FileBinaryWriter writer;
	if ( writer.Open( filename ) )
	{
		WriteToStream( writer );
	}
	writer.Close( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::Clear( )
{
	m_startVertex = 0;
	m_dataMask = 0;
	m_materialID = "";
	m_vertexes.clear( );
	m_indicies.clear( );
	m_drawInstructions.clear( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::ReadFromStream( IBinaryReader &reader )
{
	//FILE VERSION
	uint32_t version;
	reader.Read<uint32_t>( &version );
	if ( version != FILE_VERSION )
	{
		ERROR_AND_DIE( "Wrong file version! Update your file by loading the fbx and saving the mesh again." );
	}

	//material id
	reader.ReadString( &m_materialID );

	//vertex data mask
	uint32_t dataMask = ReadDataMask( reader );

	uint32_t count;

	//verticies
	m_vertexes.clear( );
	reader.Read<uint32_t>( &count );
	ReadVerticies( reader, count, dataMask );

	//indicies
	m_indicies.clear( );
	reader.Read<uint32_t>( &count );
	ReadIndicies( reader, count );

	//draw_instructions
	m_drawInstructions.clear( );
	reader.Read<uint32_t>( &count );
	ReadDrawInstructions( reader, count );
}


//-------------------------------------------------------------------------------------------------
uint32_t MeshBuilder::ReadDataMask( IBinaryReader &reader )
{
	uint32_t mask = 0;
	std::string dataMask;
	reader.ReadString( &dataMask );
	while ( strcmp( dataMask.c_str( ), "end" ) != 0 )
	{
		if ( strcmp( dataMask.c_str( ), "position" ) == 0 )
		{
			mask |= ( 1 << eMeshData_POSITION );
		}

		else if ( strcmp( dataMask.c_str( ), "color" ) == 0 )
		{
			mask |= ( 1 << eMeshData_COLOR );
		}

		else if ( strcmp( dataMask.c_str( ), "uv0" ) == 0 )
		{
			mask |= ( 1 << eMeshData_UV0 );
		}

		else if ( strcmp( dataMask.c_str( ), "uv1" ) == 0 )
		{
			mask |= ( 1 << eMeshData_UV1 );
		}

		else if ( strcmp( dataMask.c_str( ), "tangent" ) == 0 )
		{
			mask |= ( 1 << eMeshData_TANGENT );
		}

		else if ( strcmp( dataMask.c_str( ), "bitangent" ) == 0 )
		{
			mask |= ( 1 << eMeshData_BITANGENT );
		}

		else if ( strcmp( dataMask.c_str( ), "normal" ) == 0 )
		{
			mask |= ( 1 << eMeshData_NORMAL );
		}

		reader.ReadString( &dataMask );
	}

	return mask;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::ReadVerticies( IBinaryReader &reader, uint32_t count, uint32_t dataMask )
{
	m_vertexes.resize( count );
	for ( uint32_t vertIndex = 0; vertIndex < count; ++vertIndex )
	{
		if ( ( dataMask & ( BIT( eMeshData_POSITION ) ) ) != 0 )
		{
			reader.Read<Vector3f>( &( m_vertexStamp.m_position ) );
		}

		if ( ( dataMask & ( BIT( eMeshData_COLOR ) ) ) != 0 )
		{
			reader.Read<Color>( &( m_vertexStamp.m_color ) );
		}

		if ( ( dataMask & ( BIT( eMeshData_UV0 ) ) ) != 0 )
		{
			reader.Read<Vector2f>( &( m_vertexStamp.m_uv0 ) );
		}

		if ( ( dataMask & ( BIT( eMeshData_UV1 ) ) ) != 0 )
		{
			reader.Read<Vector2f>( &( m_vertexStamp.m_uv1 ) );
		}

		if ( ( dataMask & ( BIT( eMeshData_TANGENT ) ) ) != 0 )
		{
			reader.Read<Vector3f>( &( m_vertexStamp.m_tangent ) );
		}

		if ( ( dataMask & ( BIT( eMeshData_BITANGENT ) ) ) != 0 )
		{
			reader.Read<Vector3f>( &( m_vertexStamp.m_bitangent ) );
		}

		if ( ( dataMask & ( BIT( eMeshData_NORMAL ) ) ) != 0 )
		{
			reader.Read<Vector3f>( &( m_vertexStamp.m_normal ) );
		}

		m_vertexes[vertIndex] = m_vertexStamp;
	}
	m_startVertex = m_vertexes.size( );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::ReadIndicies( IBinaryReader &reader, uint32_t count )
{
	m_indicies.resize( count );
	for ( uint32_t index = 0; index < count; ++index )
	{
		reader.Read<unsigned int>( &( m_indicies[index] ) );
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::ReadDrawInstructions( IBinaryReader &reader, uint32_t count )
{
	m_drawInstructions.resize( count );
	for ( uint32_t instructionIndex = 0; instructionIndex < count; ++instructionIndex )
	{
		reader.Read<DrawInstruction>( &( m_drawInstructions[instructionIndex] ) );
	}
	m_useIBO = m_drawInstructions[0].m_useIndexBuffer;
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::WriteToStream( IBinaryWriter &writer ) const
{
	//FILE VERSION
	writer.Write<uint32_t>( FILE_VERSION );

	//material id
	writer.WriteString( GetMaterialID( ) );

	//vertex data mask
	uint32_t dataMask = GetDataMask( );
	WriteDataMask( writer, dataMask );

	//verticies
	uint32_t vertexCount = m_vertexes.size( );
	writer.Write<uint32_t>( vertexCount );
	WriteVerticies( writer, dataMask );
	
	//indicies
	uint32_t indiciesCount = m_indicies.size( );
	writer.Write<uint32_t>( indiciesCount );
	WriteIndicies( writer );

	//draw_instructions
	uint32_t drawInstructionCount = m_drawInstructions.size( );
	writer.Write<uint32_t>( drawInstructionCount );
	WriteDrawInstructions( writer );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::WriteDataMask( IBinaryWriter &writer, uint32_t dataMask ) const
{
	if ( ( dataMask & ( BIT( eMeshData_POSITION ) ) ) != 0 )
	{
		writer.WriteString( "position" );
	}

	if ( ( dataMask & ( BIT( eMeshData_COLOR ) ) ) != 0 )
	{
		writer.WriteString( "color" );
	}

	if ( ( dataMask & ( BIT( eMeshData_UV0 ) ) ) != 0 )
	{
		writer.WriteString( "uv0" );
	}

	if ( ( dataMask & ( BIT( eMeshData_UV1 ) ) ) != 0 )
	{
		writer.WriteString( "uv1" );
	}

	if ( ( dataMask & ( BIT( eMeshData_TANGENT ) ) ) != 0 )
	{
		writer.WriteString( "tangent" );
	}

	if ( ( dataMask & ( BIT( eMeshData_BITANGENT ) ) ) != 0 )
	{
		writer.WriteString( "bitangent" );
	}

	if ( ( dataMask & ( BIT( eMeshData_NORMAL ) ) ) != 0 )
	{
		writer.WriteString( "normal" );
	}

	writer.WriteString( "end" );
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::WriteVerticies( IBinaryWriter & writer, uint32_t dataMask ) const
{
	for ( uint32_t vertIndex = 0; vertIndex < m_vertexes.size( ); ++vertIndex )
	{
		if ( ( dataMask & ( BIT( eMeshData_POSITION ) ) ) != 0 )
		{
			writer.Write<Vector3f>( m_vertexes[vertIndex].m_position );
		}

		if ( ( dataMask & ( BIT( eMeshData_COLOR ) ) ) != 0 )
		{
			writer.Write<Color>( m_vertexes[vertIndex].m_color );
		}

		if ( ( dataMask & ( BIT( eMeshData_UV0 ) ) ) != 0 )
		{
			writer.Write<Vector2f>( m_vertexes[vertIndex].m_uv0 );
		}

		if ( ( dataMask & ( BIT( eMeshData_UV1 ) ) ) != 0 )
		{
			writer.Write<Vector2f>( m_vertexes[vertIndex].m_uv1 );
		}

		if ( ( dataMask & ( BIT( eMeshData_TANGENT ) ) ) != 0 )
		{
			writer.Write<Vector3f>( m_vertexes[vertIndex].m_tangent );
		}

		if ( ( dataMask & ( BIT( eMeshData_BITANGENT ) ) ) != 0 )
		{
			writer.Write<Vector3f>( m_vertexes[vertIndex].m_bitangent );
		}

		if ( ( dataMask & ( BIT( eMeshData_NORMAL ) ) ) != 0 )
		{
			writer.Write<Vector3f>( m_vertexes[vertIndex].m_normal );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::WriteIndicies( IBinaryWriter & writer ) const
{
	for ( uint32_t index = 0; index < m_indicies.size( ); ++index )
	{
		writer.Write<unsigned int>( m_indicies[index] );
	}
}


//-------------------------------------------------------------------------------------------------
void MeshBuilder::WriteDrawInstructions( IBinaryWriter & writer ) const
{
	for ( uint32_t instructionIndex = 0; instructionIndex < m_drawInstructions.size( ); ++instructionIndex )
	{
		writer.Write<DrawInstruction>( m_drawInstructions[instructionIndex] );
	}
}