#pragma once

#include <vector>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/RenderSystem/Vertex.hpp"
#include "Engine/RenderSystem/Mesh.hpp"
#include "Engine/RenderSystem/DrawInstruction.hpp"
#include "Engine/Utils/MeshBuilderDataUtils.hpp"


//-------------------------------------------------------------------------------------------------
enum eMeshDataFlag
{
	eMeshData_POSITION = 0,
	eMeshData_COLOR,
	eMeshData_UV0,
	eMeshData_UV1,
	eMeshData_TANGENT,
	eMeshData_BITANGENT,
	eMeshData_NORMAL,
	eMeshData_BONEWEIGHTS,
};


//-------------------------------------------------------------------------------------------------
class Skeleton;
class IBinaryReader;
class IBinaryWriter;
class AABB2f;


//-------------------------------------------------------------------------------------------------
class MeshBuilder
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	// 1: Initial Version
	static uint32_t const FILE_VERSION = 1;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static std::vector<MeshBuilder*> CombineAllWithSameMaterialID( std::vector<MeshBuilder*> const & meshBuilders );
	static MeshBuilder * CreateSkeletonMesh( Skeleton const * skeleton );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	bool m_useIBO;
	int m_startVertex;
	uint32_t m_dataMask;
	std::string m_materialID;
	Vertex_Master m_vertexStamp;
	DrawInstruction m_instructionStamp;
	std::vector<Vertex_Master> m_vertexes;
	std::vector<unsigned int> m_indicies;
	std::vector<DrawInstruction> m_drawInstructions;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	MeshBuilder( bool useIBO = true );
	~MeshBuilder( );

	void Begin( );
	void End( );

	void SetPrimitiveType( ePrimitiveType const & pType );
	void SetColor( Color const &color );
	void SetTangent( Vector3f const &tangent );
	void SetBitangent( Vector3f const &bitangent );
	void SetNormal( Vector3f const &normal );
	void SetTBN( Vector3f const &tangent, Vector3f const &bitangent, Vector3f const &normal );
	void SetUV0( Vector2f const &uv );
	void SetUV0( float u, float v );
	void SetUV1( Vector2f const &uv );
	void SetUV1( float u, float v );
	void SetBoneWeights( Vector4i const & boneIndicies, Vector4f const & boneWeights );

	void AddVertex( Vector3f const &position );
	void AddTriangle( Vector3f const & topLeft, Vector3f const & bottomLeft, Vector3f const & bottomRight );
	void AddQuad( Vector3f const & topLeft, Vector3f const & bottomRight );
	void AddQuad( Vector3f const & topLeft, Vector3f const & bottomRight, AABB2f const & uvBounds );
	void AddQuad( Vector3f const & topLeft, Vector3f const & bottomLeft, Vector3f const & bottomRight, Vector3f const & topRight );
	void AddQuad( Vector3f const & topLeft, Vector3f const & bottomLeft, Vector3f const & bottomRight, Vector3f const & topRight,
		Vector2f const & uvTopLeft, Vector2f const & uvBottomLeft, Vector2f const & uvBottomRight, Vector2f const & uvTopRight );
	void AddPlane( Vector3f const &pos, Vector3f const &right, Vector3f const &up,
		float xStart, float xEnd, int xSections,
		float yStart, float yEnd, int ySections );
	void AddCustom( int effectIndex,
		float xStart, float xEnd, int xSections,
		float yStart, float yEnd, int ySections );
	void AddPatch( PatchFunc *patchFunc, void const *user_data,
		float xStart, float xEnd, int xSections,
		float yStart, float yEnd, int ySections );
	void AddCube( Vector3f const & pos, float size = 1.f );
	void AddLine( Vector3f const & start, Vector3f const & end );
	void AddIndex( unsigned int index );
	void AddQuadIndicies( unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br );

	std::vector<DrawInstruction> const & GetDrawInstructions( ) const { return m_drawInstructions; }
	std::vector<Vertex_Master> const & GetVertexData( ) const { return m_vertexes; }
	std::vector<unsigned int> const & GetIndexBuffer( ) const { return m_indicies; }
	int GetCurrentIndex( ) const { return m_startVertex; }
	std::string const & GetMaterialID( ) const { return m_materialID; }
	uint32_t GetDataMask( ) const { return m_dataMask; }
	uint32_t GetNextIndex( ) const;
	bool DoesUseIBO( ) const { return m_useIBO; }
	void ClearBoneWeights( );

	void MeshReduction( );
	void ReadFromFile( std::string const &filename );
	void WriteToFile( std::string const &filename ) const;
	void Clear( );

private:
	void ReadFromStream( IBinaryReader &reader );
	uint32_t ReadDataMask( IBinaryReader &reader );
	void ReadVerticies( IBinaryReader &reader, uint32_t count, uint32_t dataMask );
	void ReadIndicies( IBinaryReader &reader, uint32_t count );
	void ReadDrawInstructions( IBinaryReader &reader, uint32_t count );

	void WriteToStream( IBinaryWriter &writer ) const;
	void WriteDataMask( IBinaryWriter &writer, uint32_t dataMask ) const;
	void WriteVerticies( IBinaryWriter &writer, uint32_t dataMask ) const;
	void WriteIndicies( IBinaryWriter &writer ) const;
	void WriteDrawInstructions( IBinaryWriter &writer ) const;
};