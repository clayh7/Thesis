#include "Engine/RenderSystem/Mesh.hpp"

#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/BitmapFont.hpp"
#include "Engine/RenderSystem/Glyph.hpp"
#include "Engine/RenderSystem/Kerning.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC std::vector<Mesh*, UntrackedAllocator<Mesh*>> Mesh::s_defaultMeshes;


//-------------------------------------------------------------------------------------------------
STATIC void Mesh::InitializeDefaultMeshes( )
{
	for( int meshIndex = 0; meshIndex < eMeshShape_COUNT; ++meshIndex )
	{
		s_defaultMeshes.push_back( new Mesh( (eMeshShape) meshIndex ) );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void Mesh::DestroyDefaultMeshes( )
{
	for( int meshIndex = 0; meshIndex < eMeshShape_COUNT; ++meshIndex )
	{
		delete s_defaultMeshes[meshIndex];
		s_defaultMeshes[meshIndex] = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC Mesh const * Mesh::GetMeshShape( eMeshShape const & meshShape )
{
	return s_defaultMeshes[meshShape];
}


//-------------------------------------------------------------------------------------------------
Mesh::Mesh( eVertexType const & vertexType )
	: m_vboID( NULL )
	, m_iboID( NULL )
	, m_vertexType( vertexType )
{
	SetVertexLayout( vertexType );
}


//-------------------------------------------------------------------------------------------------
Mesh::Mesh( )
	: Mesh( eVertexType_PCU )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Mesh::Mesh( eMeshShape const & shape )
	: m_vboID( NULL )
	, m_iboID( NULL )
	//, m_vertexType( vertexType )
{
	//Create object
	switch ( shape )
	{//Remember Tex Coords: TL(0,0) BR(1,1)
	case eMeshShape_QUAD:
	{
		//Vertex Definition Layout
		SetVertexLayout( eVertexType_PCUTB );
		std::vector<Vertex_PCUTB> vertexes;

		//Setup Vertex
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, +0.0f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, +0.0f ), Color::WHITE, Vector2f( 1.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, +0.0f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, +0.0f ), Color::WHITE, Vector2f( 0.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topLeft

		//Create VBO
		m_vboID = g_RenderSystem->CreateRenderBuffer( &vertexes[0], vertexes.size( ), sizeof( Vertex_PCUTB ) );

		//Create IBO
		uint32_t indicies[] =
		{
			0, 1, 2,
			0, 2, 3
		};
		int count = 2 * 3;
		m_iboID = g_RenderSystem->CreateRenderBuffer( &indicies, count, sizeof( indicies[0] ) );

		m_drawInstructions.push_back( DrawInstruction( true, ePrimitiveType_TRIANGLES, 0, count ) );
		break;
	}
	case eMeshShape_CUBE_OUTLINE:
		//#TODO: fix
	case eMeshShape_CUBE:
	{
		//Vertex Definition Layout
		SetVertexLayout( eVertexType_PCUTB );
		std::vector<Vertex_PCUTB> vertexes;

		//#TODO: Use IBOs
		//Front (two triangles each side)
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topLeft

		//Back
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( -1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 1.f ), Vector3f( -1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( -1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( -1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( -1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 0.f ), Vector3f( -1.f, 0.f, 0.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topRight

		//Left
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 0.f, 0.f, 1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 1.f ), Vector3f( 0.f, 0.f, 1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 0.f, 0.f, 1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topLeft

		//Right
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 0.f, 0.f, -1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 1.f ), Vector3f( 0.f, 0.f, -1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 0.f, 0.f, -1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ), Vector3f( 0.f, 1.f, 0.f ) ) );	//topRight

		//Top
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ) ) );	//topRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ) ) );	//topRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, +0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, -1.f ) ) );	//topLeft

		//Bottom
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 1.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ) ) );	//bottomLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ) ) );	//topLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, -0.5f ), Color::WHITE, Vector2f( 0.f, 1.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ) ) );	//bottomRight
		vertexes.push_back( Vertex_PCUTB( Vector3f( +0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 1.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ) ) );	//topLeft
		vertexes.push_back( Vertex_PCUTB( Vector3f( -0.5f, -0.5f, +0.5f ), Color::WHITE, Vector2f( 0.f, 0.f ), Vector3f( 1.f, 0.f, 0.f ), Vector3f( 0.f, 0.f, 1.f ) ) );	//topRight
		
		//Create VBO
		int count = vertexes.size( );
		m_vboID = g_RenderSystem->CreateRenderBuffer( &vertexes[0], count, sizeof( Vertex_PCUTB ) );

		//No IBO
		m_drawInstructions.push_back( DrawInstruction( false, ePrimitiveType_TRIANGLES, 0, count ) );
		break;
	}
	case eMeshShape_SPHERE_OUTLINE:
		//#TODO: Fix
	case eMeshShape_SPHERE: //#TODO: Redo with less vertexes
	{
		//Vertex Definition Layout
		SetVertexLayout( eVertexType_PCUTB );
		std::vector<Vertex_PCUTB> vertexes;

		Vertex_PCUTB bottomLeft = Vertex_PCUTB( );
		Vertex_PCUTB bottomRight = Vertex_PCUTB( );
		Vertex_PCUTB topRight = Vertex_PCUTB( );
		Vertex_PCUTB topLeft = Vertex_PCUTB( );

		std::vector<unsigned int> indicies;
		unsigned int vertIndex = 0;

		int horizontalStrips = 20;
		int verticalStrips = 20;
		float radius = 0.5f;

		float deltaPitch = 180.f / ( float ) horizontalStrips;
		float deltaYaw = 360.f / ( float ) verticalStrips;
		for ( int horizontalStripCount = 0; horizontalStripCount < horizontalStrips; ++horizontalStripCount )
		{
			float currentPitch = deltaPitch * ( float ) horizontalStripCount;
			float nextPitch = deltaPitch * ( float ) ( horizontalStripCount + 1 );

			float topYPos = radius * CosDegrees( currentPitch );
			float bottomYPos = radius * CosDegrees( nextPitch );

			float heightRadius = radius * SinDegrees( currentPitch );
			float nextHeightRadius = radius * SinDegrees( nextPitch );

			for ( int verticalStripCount = 0; verticalStripCount < verticalStrips; ++verticalStripCount )
			{
				float currentYaw = deltaYaw * ( float ) verticalStripCount;
				float nextYaw = deltaYaw * ( float ) ( verticalStripCount + 1 );

				//---------------------------------------------------------------------------------------------
				// World Coords
				//---------------------------------------------------------------------------------------------
				float rightXPosTop = heightRadius * CosDegrees( currentYaw );
				float leftXPosTop = heightRadius * CosDegrees( nextYaw );
				float rightZPosTop = heightRadius * -SinDegrees( currentYaw );
				float leftZPosTop = heightRadius * -SinDegrees( nextYaw );

				float rightXPosBot = nextHeightRadius * CosDegrees( currentYaw );
				float leftXPosBot = nextHeightRadius * CosDegrees( nextYaw );
				float rightZPosBot = nextHeightRadius * -SinDegrees( currentYaw );
				float leftZPosBot = nextHeightRadius * -SinDegrees( nextYaw );

				topRight.m_position = Vector3f( rightXPosTop, topYPos, rightZPosTop );
				topLeft.m_position = Vector3f( leftXPosTop, topYPos, leftZPosTop );
				bottomRight.m_position = Vector3f( rightXPosBot, bottomYPos, rightZPosBot );
				bottomLeft.m_position = Vector3f( leftXPosBot, bottomYPos, leftZPosBot );

				//---------------------------------------------------------------------------------------------
				// Tangent & BiTangents
				//---------------------------------------------------------------------------------------------
				Vector3f TRtangent( -SinDegrees( currentYaw ), 0.f, -CosDegrees( currentYaw ) );
				Vector3f TRbitangent( -CosDegrees( currentYaw ) * CosDegrees( currentPitch ), SinDegrees( currentPitch ), SinDegrees( currentYaw ) * CosDegrees( currentPitch ) );
				Vector3f TLtangent( -SinDegrees( nextYaw ), 0.f, -CosDegrees( nextYaw ) );
				Vector3f TLbitangent( -CosDegrees( nextYaw ) * CosDegrees( currentPitch ), SinDegrees( currentPitch ), SinDegrees( nextYaw ) * CosDegrees( currentPitch ) );
				Vector3f BRtangent( -SinDegrees( currentYaw ), 0.f, -CosDegrees( currentYaw ) );
				Vector3f BRbitangent( -CosDegrees( currentYaw ) * CosDegrees( nextPitch ), SinDegrees( nextPitch ), SinDegrees( currentYaw ) * CosDegrees( nextPitch ) );
				Vector3f BLtangent( -SinDegrees( nextYaw ), 0.f, -CosDegrees( nextYaw ) );
				Vector3f BLbitangent( -CosDegrees( nextYaw ) * CosDegrees( nextPitch ), SinDegrees( nextPitch ), SinDegrees( nextYaw ) * CosDegrees( nextPitch ) );

				topRight.m_tangent = TRtangent;
				topRight.m_bitangent = TRbitangent;
				topLeft.m_tangent = TLtangent;
				topLeft.m_bitangent = TLbitangent;
				bottomRight.m_tangent = BRtangent;
				bottomRight.m_bitangent = BRbitangent;
				bottomLeft.m_tangent = BLtangent;
				bottomLeft.m_bitangent = BLbitangent;

				//---------------------------------------------------------------------------------------------
				// Tex Coords
				//---------------------------------------------------------------------------------------------
				//Top Left 0,0 Bottom Right 1,1
				float leftTexCoordX = ( float ) ( verticalStripCount + 1 ) / ( float ) verticalStrips;
				float rightTexCoordX = ( float ) verticalStripCount / ( float ) verticalStrips;
				float topTexCoordY = ( float ) horizontalStripCount / ( float ) horizontalStrips;
				float bottomTexCoordY = ( float ) ( horizontalStripCount + 1 ) / ( float ) horizontalStrips;
				topRight.m_uv = Vector2f( rightTexCoordX, topTexCoordY );
				bottomLeft.m_uv = Vector2f( leftTexCoordX, bottomTexCoordY );
				topLeft.m_uv = Vector2f( leftTexCoordX, topTexCoordY );
				bottomRight.m_uv = Vector2f( rightTexCoordX, bottomTexCoordY );

				//---------------------------------------------------------------------------------------------
				// Adding them to the Buffers
				//---------------------------------------------------------------------------------------------
				vertexes.push_back( bottomLeft );
				vertexes.push_back( bottomRight );
				vertexes.push_back( topRight );
				vertexes.push_back( topLeft );

				indicies.push_back( vertIndex + 2 );
				indicies.push_back( vertIndex + 1 );
				indicies.push_back( vertIndex + 0 );
				indicies.push_back( vertIndex + 3 );
				indicies.push_back( vertIndex + 2 );
				indicies.push_back( vertIndex + 0 );
				vertIndex += 4;
			}
		}

		//Create VBO
		g_RenderSystem->CreateOrUpdateRenderBuffer( &m_vboID, &vertexes[0], vertexes.size( ), sizeof( Vertex_PCUTB ) );

		//Create IBO
		g_RenderSystem->CreateOrUpdateRenderBuffer( &m_iboID, &indicies[0], indicies.size( ), sizeof( indicies[0] ) );

		m_drawInstructions.push_back( DrawInstruction( true, ePrimitiveType_TRIANGLES, 0, indicies.size( ) ) );
		break;
	}
	case eMeshShape_AXIS:
	{
		//Vertex Definition Layout
		SetVertexLayout( eVertexType_PC );
		std::vector<Vertex_PC> vertexes;

		vertexes.push_back( Vertex_PC( Vector3f( 0.f ), Color::RED ) );
		vertexes.push_back( Vertex_PC( Vector3f( +1.0f, +0.0f, +0.0f ), Color::RED ) );
		vertexes.push_back( Vertex_PC( Vector3f( 0.f ), Color::GREEN ) );
		vertexes.push_back( Vertex_PC( Vector3f( +0.0f, +1.0f, +0.0f ), Color::GREEN ) );
		vertexes.push_back( Vertex_PC( Vector3f( 0.f ), Color::BLUE ) );
		vertexes.push_back( Vertex_PC( Vector3f( +0.0f, +0.0f, +1.0f ), Color::BLUE ) );

		//Create VBO
		int count = vertexes.size( );
		m_vboID = g_RenderSystem->CreateRenderBuffer( &vertexes[0], count, sizeof( Vertex_PC ) );

		//No IBO
		m_drawInstructions.push_back( DrawInstruction( false, ePrimitiveType_LINES, 0, count ) );
		break;
	}
	case eMeshShape_POINT:
	{
		//Vertex Definition Layout
		SetVertexLayout( eVertexType_PC );
		std::vector<Vertex_PC> vertexes;

		vertexes.push_back( Vertex_PC( Vector3f( -0.5f, +0.0f, +0.0f ), Color::WHITE ) );
		vertexes.push_back( Vertex_PC( Vector3f( +0.5f, +0.0f, +0.0f ), Color::WHITE ) );
		vertexes.push_back( Vertex_PC( Vector3f( +0.0f, -0.5f, +0.0f ), Color::WHITE ) );
		vertexes.push_back( Vertex_PC( Vector3f( +0.0f, +0.5f, +0.0f ), Color::WHITE ) );
		vertexes.push_back( Vertex_PC( Vector3f( +0.0f, +0.0f, -0.5f ), Color::WHITE ) );
		vertexes.push_back( Vertex_PC( Vector3f( +0.0f, +0.0f, +0.5f ), Color::WHITE ) );

		//Create VBO
		int count = vertexes.size( );
		m_vboID = g_RenderSystem->CreateRenderBuffer( &vertexes[0], count, sizeof( Vertex_PC ) );

		//No IBO
		m_drawInstructions.push_back( DrawInstruction( false, ePrimitiveType_LINES, 0, count ) );
		break;
	}
	default:
		ERROR_AND_DIE( "Shape Mesh not implemented." )
		break;
	}
}


//-------------------------------------------------------------------------------------------------
Mesh::Mesh( std::string const & textString, float scale /*= 12.f */, BitmapFont const * font /*= nullptr*/ )
	: m_vboID( NULL )
	, m_iboID( NULL )
	//, m_vertexType( vertexType )
{
	//UpdateText( textString, scale, font );
	std::string asciiText = textString;
	if ( asciiText.size() <= 0 )
	{
		asciiText.append( " " );
	}
	ASSERT_RECOVERABLE( asciiText.size() > 0, "Can only create mesh for strings at least the size of 1." )

	//Vertex Definition Layout
	SetVertexLayout( eVertexType_PCU );
	std::vector<Vertex_PCU> vertexes;
	std::vector<unsigned int> indicies;
	unsigned int vertIndex = 0;

	Vector3f origin( Vector3f::ZERO );
	Vector3f rightVector( Vector3f( 1.f, 0.f, 0.f ) );
	Vector3f upVector( Vector3f( 0.f, 1.f, 0.f ) );

	if ( font == nullptr )
		font = g_RenderSystem->GetDefaultFont();

	scale /= ( float ) font->GetFontSize( );
	scale /= ( float ) 200.f;

	Glyph const *previousGlyph = nullptr;
	Glyph const *undefinedGlyph = font->GetGlyph( '?' );

	for ( char const &glyphToDraw : asciiText )
	{
		Glyph const *glyph = font->GetGlyph( glyphToDraw );

		//Unknown characters
		if ( glyph == nullptr )
		{
			glyph = undefinedGlyph;
		}

		//Adjust for kerning
		Kerning const & kerning = font->GetKerning( previousGlyph, glyph );
		origin += rightVector * ( kerning.m_amount * scale );

		Vector2i const offset = glyph->GetOffset( );
		Vector2i const size = glyph->GetSize( );

		//Coords for Quad
		Vector3f const topLeft = origin
			+ rightVector * ( float ) offset.x * scale
			- upVector * ( float ) offset.y * scale;

		Vector3f const bottomLeft = topLeft
			- upVector * ( float ) size.y * scale;

		Vector3f const topRight = topLeft
			+ rightVector * ( float ) size.x * scale;

		Vector3f const bottomRight = bottomLeft
			+ rightVector * ( float ) size.x * scale;

		//AABB2 for uvs
		AABB2f const uvTexCoords = glyph->GetTexCoords( );

		vertexes.push_back( Vertex_PCU( bottomLeft, Color::WHITE, Vector2f( uvTexCoords.mins.x, uvTexCoords.maxs.y ) ) );
		vertexes.push_back( Vertex_PCU( bottomRight, Color::WHITE, uvTexCoords.maxs ) );
		vertexes.push_back( Vertex_PCU( topRight, Color::WHITE, Vector2f( uvTexCoords.maxs.x, uvTexCoords.mins.y ) ) );
		vertexes.push_back( Vertex_PCU( topLeft, Color::WHITE, uvTexCoords.mins ) );

		indicies.push_back( vertIndex + 0 );
		indicies.push_back( vertIndex + 1 );
		indicies.push_back( vertIndex + 2 );
		indicies.push_back( vertIndex + 0 );
		indicies.push_back( vertIndex + 2 );
		indicies.push_back( vertIndex + 3 );
		vertIndex += 4;

		//Move origin for next glyph
		origin += rightVector * ( glyph->m_xadvance * scale );

		previousGlyph = glyph;
	}

	//Create VBO
	m_vboID = g_RenderSystem->CreateRenderBuffer( &vertexes[0], vertexes.size( ), sizeof( Vertex_PCU ) );

	//Create IBO
	int count = indicies.size( );
	m_iboID = g_RenderSystem->CreateRenderBuffer( &indicies[0], count, sizeof( indicies[0] ) );

	m_drawInstructions.push_back( DrawInstruction( true, ePrimitiveType_TRIANGLES, 0, count ) );
}


//-------------------------------------------------------------------------------------------------
Mesh::Mesh( MeshBuilder const * meshBuilder, eVertexType const & vertexType )
	: m_vboID( NULL )
	, m_iboID( NULL )
	, m_vertexType( vertexType )
{
	SetVertexLayout( vertexType );

	m_drawInstructions.clear( );
	m_drawInstructions = meshBuilder->GetDrawInstructions( );

	std::vector<Vertex_Master> vertexData = meshBuilder->GetVertexData( );
	int vertexCount = vertexData.size( );

	int vertexSize = GetVertexSize();
	int vertexBufferSize = vertexCount * vertexSize;
	byte_t * vertexBuffer = new byte_t[vertexBufferSize];
	byte_t * dest = vertexBuffer;

	switch ( vertexType )
	{
	case eVertexType_PC:
		for ( Vertex_Master vert : vertexData )
		{
			Vertex_PC parsed( vert.m_position, vert.m_color );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCU:
		for ( Vertex_Master vert : vertexData )
		{
			Vertex_PCU parsed( vert.m_position, vert.m_color, vert.m_uv0 );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCUN:
		for ( Vertex_Master vert : vertexData )
		{
			Vertex_PCUN parsed( vert.m_position, vert.m_color, vert.m_uv0, vert.m_normal );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCUTB:
		for( Vertex_Master vert : vertexData )
		{
			Vertex_PCUTB parsed( vert.m_position, vert.m_color, vert.m_uv0, vert.m_tangent, vert.m_bitangent );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCUTBWI:
		for( Vertex_Master vert : vertexData )
		{
			Vertex_PCUTBWI parsed( vert.m_position, vert.m_color, vert.m_uv0, vert.m_tangent, vert.m_bitangent, vert.m_boneWeights, vert.m_boneIndicies );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	default:
		ERROR_AND_DIE( "VertexType not implemented" );
		break;
	}

	//Create VBO
	m_vboID = g_RenderSystem->CreateRenderBuffer( vertexBuffer, vertexCount, vertexSize );
	delete vertexBuffer;

	if ( meshBuilder->DoesUseIBO( ) )
	{
		std::vector<unsigned int> const & indexBuffer = meshBuilder->GetIndexBuffer( );
		m_iboID = g_RenderSystem->CreateRenderBuffer( &indexBuffer[0], indexBuffer.size(), sizeof( indexBuffer[0] ) );
	}
}


//-------------------------------------------------------------------------------------------------
Mesh::~Mesh( )
{
	glDeleteBuffers( 1, &m_vboID );
	if ( m_iboID != NULL )
	{
		glDeleteBuffers( 1, &m_iboID );
	}
}


//-------------------------------------------------------------------------------------------------
std::vector< VertexDefinition > const & Mesh::GetLayout( ) const
{
	return m_layout;
}


//-------------------------------------------------------------------------------------------------
std::vector< DrawInstruction > const & Mesh::GetDrawInstructions( ) const
{
	return m_drawInstructions;
}


//-------------------------------------------------------------------------------------------------
int Mesh::GetVertexSize( ) const
{
	return m_layout[0].m_stride;
}


//-------------------------------------------------------------------------------------------------
void Mesh::SetVertexLayout( eVertexType const & vertexType )
{
	m_layout.clear( );
	switch ( vertexType )
	{
	case eVertexType_PC:
		m_layout.push_back( VertexDefinition( "inPosition", 0, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PC ), offsetof( Vertex_PC, m_position ) ) );
		m_layout.push_back( VertexDefinition( "inColor", 1, 4, VertexDataType_UBYTE, true, sizeof( Vertex_PC ), offsetof( Vertex_PC, m_color ) ) );
		break;
	case eVertexType_PCU:
		m_layout.push_back( VertexDefinition( "inPosition", 0, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCU ), offsetof( Vertex_PCU, m_position ) ) );
		m_layout.push_back( VertexDefinition( "inColor", 1, 4, VertexDataType_UBYTE, true, sizeof( Vertex_PCU ), offsetof( Vertex_PCU, m_color ) ) );
		m_layout.push_back( VertexDefinition( "inUV0", 2, 2, VertexDataType_FLOAT, false, sizeof( Vertex_PCU ), offsetof( Vertex_PCU, m_uv ) ) );
		break;
	case eVertexType_PCUN:
		m_layout.push_back( VertexDefinition( "inPosition", 0, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUN ), offsetof( Vertex_PCUN, m_position ) ) );
		m_layout.push_back( VertexDefinition( "inColor", 1, 4, VertexDataType_UBYTE, true, sizeof( Vertex_PCUN ), offsetof( Vertex_PCUN, m_color ) ) );
		m_layout.push_back( VertexDefinition( "inUV0", 2, 2, VertexDataType_FLOAT, false, sizeof( Vertex_PCUN ), offsetof( Vertex_PCUN, m_uv ) ) );
		m_layout.push_back( VertexDefinition( "inNormal", 3, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUN ), offsetof( Vertex_PCUN, m_normal ) ) );
		break;
	case eVertexType_PCUTB:
		m_layout.push_back( VertexDefinition( "inPosition", 0, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTB ), offsetof( Vertex_PCUTB, m_position ) ) );
		m_layout.push_back( VertexDefinition( "inColor", 1, 4, VertexDataType_UBYTE, true, sizeof( Vertex_PCUTB ), offsetof( Vertex_PCUTB, m_color ) ) );
		m_layout.push_back( VertexDefinition( "inUV0", 2, 2, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTB ), offsetof( Vertex_PCUTB, m_uv ) ) );
		m_layout.push_back( VertexDefinition( "inTangent", 3, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTB ), offsetof( Vertex_PCUTB, m_tangent ) ) );
		m_layout.push_back( VertexDefinition( "inBiTangent", 4, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTB ), offsetof( Vertex_PCUTB, m_bitangent ) ) );
		break;
	case eVertexType_PCUTBWI:
		m_layout.push_back( VertexDefinition( "inPosition", 0, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTBWI ), offsetof( Vertex_PCUTBWI, m_position ) ) );
		m_layout.push_back( VertexDefinition( "inColor", 1, 4, VertexDataType_UBYTE, true, sizeof( Vertex_PCUTBWI ), offsetof( Vertex_PCUTBWI, m_color ) ) );
		m_layout.push_back( VertexDefinition( "inUV0", 2, 2, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTBWI ), offsetof( Vertex_PCUTBWI, m_uv ) ) );
		m_layout.push_back( VertexDefinition( "inTangent", 3, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTBWI ), offsetof( Vertex_PCUTBWI, m_tangent ) ) );
		m_layout.push_back( VertexDefinition( "inBiTangent", 4, 3, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTBWI ), offsetof( Vertex_PCUTBWI, m_bitangent ) ) );
		m_layout.push_back( VertexDefinition( "inBoneWeight", 5, 4, VertexDataType_FLOAT, false, sizeof( Vertex_PCUTBWI ), offsetof( Vertex_PCUTBWI, m_boneWeights ) ) );
		m_layout.push_back( VertexDefinition( "inBoneIndex", 6, 4, VertexDataType_UINT, false, sizeof( Vertex_PCUTBWI ), offsetof( Vertex_PCUTBWI, m_boneIndicies ) ) );
		break;
	case eVertexType_SPRITE:
		m_layout.push_back( VertexDefinition( "inPosition", 0, 3, VertexDataType_FLOAT, false, sizeof( Vertex_SPRITE ), offsetof( Vertex_SPRITE, m_position ) ) );
		m_layout.push_back( VertexDefinition( "inColor", 1, 4, VertexDataType_UBYTE, true, sizeof( Vertex_SPRITE ), offsetof( Vertex_SPRITE, m_color ) ) );
		m_layout.push_back( VertexDefinition( "inUV0", 2, 2, VertexDataType_FLOAT, false, sizeof( Vertex_SPRITE ), offsetof( Vertex_SPRITE, m_uv ) ) );
		break;
	}
}


//-------------------------------------------------------------------------------------------------
Vector3f Mesh::Update( std::string const & newText, float scale /*= 12.f*/, BitmapFont const * font /*= nullptr */ )
{
	std::string asciiText = newText;
	if ( asciiText.size( ) <= 0 )
	{
		asciiText.append( " " );
	}
	ASSERT_RECOVERABLE( asciiText.size( ) > 0, "Can only create mesh for strings at least the size of 1." )

	//Vertex Definition Layout
	SetVertexLayout( eVertexType_PCU );
	std::vector<Vertex_PCU> vertexes;
	std::vector<unsigned int> indicies;
	unsigned int vertIndex = 0;

	Vector3f origin( Vector3f::ZERO );
	Vector3f rightVector( Vector3f( 1.f, 0.f, 0.f ) );
	Vector3f upVector( Vector3f( 0.f, 1.f, 0.f ) );

	if ( font == nullptr )
		font = g_RenderSystem->GetDefaultFont( );

	scale /= ( float ) font->GetFontSize( );

	//This magic number is just to make the size "12" feel like ~12
	//scale *= 2.5f;
	scale *= 1.65f;
	//scale /= ( float ) 200.f;

	Glyph const * previousGlyph = nullptr;
	Glyph const * undefinedGlyph = font->GetGlyph( '?' );

	Vector3f maxSize = Vector3f::ZERO;
	Vector3f minSize = Vector3f::ZERO;
	for ( char const &glyphToDraw : asciiText )
	{
		Glyph const *glyph = font->GetGlyph( glyphToDraw );

		//Unknown characters
		if ( glyph == nullptr )
		{
			glyph = undefinedGlyph;
		}

		//Adjust for kerning
		Kerning const kerning = font->GetKerning( previousGlyph, glyph );
		origin += rightVector * ( kerning.m_amount * scale );

		Vector2i const offset = glyph->GetOffset( );
		Vector2i const size = glyph->GetSize( );

		//Coords for Quad
		Vector3f const topLeft = origin
			+ rightVector * ( float ) offset.x * scale
			- upVector * ( float ) offset.y * scale;

		Vector3f const bottomLeft = topLeft
			- upVector * ( float ) size.y * scale;

		Vector3f const topRight = topLeft
			+ rightVector * ( float ) size.x * scale;

		Vector3f const bottomRight = bottomLeft
			+ rightVector * ( float ) size.x * scale;

		minSize.x = Min( bottomRight.x, minSize.x );
		minSize.y = Min( bottomRight.y, minSize.y );
		minSize.z = Min( bottomRight.z, minSize.z );
		maxSize.x = Max( bottomRight.x, maxSize.x );
		maxSize.y = Max( bottomRight.y, maxSize.y );
		maxSize.z = Max( bottomRight.z, maxSize.z );

		//AABB2 for uvs
		AABB2f const uvTexCoords = glyph->GetTexCoords( );

		vertexes.push_back( Vertex_PCU( bottomLeft, Color::WHITE, Vector2f( uvTexCoords.mins.x, uvTexCoords.maxs.y ) ) );
		vertexes.push_back( Vertex_PCU( bottomRight, Color::WHITE, uvTexCoords.maxs ) );
		vertexes.push_back( Vertex_PCU( topRight, Color::WHITE, Vector2f( uvTexCoords.maxs.x, uvTexCoords.mins.y ) ) );
		vertexes.push_back( Vertex_PCU( topLeft, Color::WHITE, uvTexCoords.mins ) );

		indicies.push_back( vertIndex + 0 );
		indicies.push_back( vertIndex + 1 );
		indicies.push_back( vertIndex + 2 );
		indicies.push_back( vertIndex + 0 );
		indicies.push_back( vertIndex + 2 );
		indicies.push_back( vertIndex + 3 );
		vertIndex += 4;

		//Move origin for next glyph
		origin += rightVector * ( glyph->m_xadvance * scale );

		previousGlyph = glyph;
	}

	//Update VBO
	g_RenderSystem->UpdateRenderBuffer( m_vboID, &vertexes[0], vertexes.size( ), sizeof( Vertex_PCU ), GL_DYNAMIC_DRAW );

	//Create IBO
	int count = indicies.size( );
	g_RenderSystem->UpdateRenderBuffer(m_iboID, &indicies[0], count, sizeof( indicies[0] ), GL_DYNAMIC_DRAW );

	m_drawInstructions.clear( );
	m_drawInstructions.push_back( DrawInstruction( true, ePrimitiveType_TRIANGLES, 0, count ) );

	return maxSize - minSize;
}


//-------------------------------------------------------------------------------------------------
void Mesh::Update( MeshBuilder const * meshBuilder )
{
	m_drawInstructions.clear( );
	m_drawInstructions = meshBuilder->GetDrawInstructions( );

	std::vector<Vertex_Master> const & vertexData = meshBuilder->GetVertexData( );
	int vertexCount = vertexData.size( );

	int vertexSize = GetVertexSize( );
	int vertexBufferSize = vertexCount * vertexSize;

	//alloca allocates on the stack, automatically freed
	byte_t *vertexBuffer = (byte_t*) _alloca( vertexBufferSize );
	byte_t *dest = vertexBuffer;

	switch ( m_vertexType )
	{
	case eVertexType_PC:
		for ( Vertex_Master vert : vertexData )
		{
			Vertex_PC parsed( vert.m_position, vert.m_color );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCU:
		for ( Vertex_Master vert : vertexData )
		{
			Vertex_PCU parsed( vert.m_position, vert.m_color, vert.m_uv0 );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCUN:
		for ( Vertex_Master vert : vertexData )
		{
			Vertex_PCUN parsed( vert.m_position, vert.m_color, vert.m_uv0, vert.m_normal );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCUTB:
		for( Vertex_Master vert : vertexData )
		{
			Vertex_PCUTB parsed( vert.m_position, vert.m_color, vert.m_uv0, vert.m_tangent, vert.m_bitangent );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_PCUTBWI:
		for( Vertex_Master vert : vertexData )
		{
			Vertex_PCUTBWI parsed( vert.m_position, vert.m_color, vert.m_uv0, vert.m_tangent, vert.m_bitangent, vert.m_boneWeights, vert.m_boneIndicies );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	case eVertexType_SPRITE:
		for( Vertex_Master vert : vertexData )
		{
			Vertex_SPRITE parsed( vert.m_position, vert.m_color, vert.m_uv0 );
			memcpy( dest, &parsed, vertexSize );
			dest += vertexSize;
		}
		break;
	default:
		ERROR_AND_DIE( "VertexType not implemented" );
		break;
	}

	//Update VBO
	g_RenderSystem->CreateOrUpdateRenderBuffer( &m_vboID, vertexBuffer, vertexCount, vertexSize, GL_DYNAMIC_DRAW );

	//Update IBO
	if ( meshBuilder->DoesUseIBO( ) )
	{
		std::vector<unsigned int> const & indexBuffer = meshBuilder->GetIndexBuffer( );
		g_RenderSystem->CreateOrUpdateRenderBuffer( &m_iboID, &indexBuffer[0], indexBuffer.size( ), sizeof( indexBuffer[0] ), GL_DYNAMIC_DRAW );
	}
}