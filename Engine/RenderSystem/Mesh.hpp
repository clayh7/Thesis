#pragma once

#include <vector>
#include "Engine/Memory/MemoryAnalytics.hpp"
#include "Engine/RenderSystem/Vertex.hpp"
#include "Engine/RenderSystem/VertexDefinition.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"


//-------------------------------------------------------------------------------------------------
class BitmapFont;
class DrawInstruction;
class MeshBuilder;


//-------------------------------------------------------------------------------------------------
enum eMeshShape
{
	eMeshShape_QUAD,
	eMeshShape_CUBE,
	eMeshShape_CUBE_OUTLINE,
	eMeshShape_SPHERE,
	eMeshShape_SPHERE_OUTLINE,
	eMeshShape_AXIS,
	eMeshShape_POINT,
	eMeshShape_COUNT,
};


//-------------------------------------------------------------------------------------------------
class Mesh
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	//Static Memory Allocation
	static std::vector<Mesh*, UntrackedAllocator<Mesh*>> s_defaultMeshes;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void InitializeDefaultMeshes( );
	static void DestroyDefaultMeshes( );
	static Mesh const * GetMeshShape( eMeshShape const & meshShape );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	unsigned int m_vboID;
	unsigned int m_iboID;
	eVertexType m_vertexType;
	std::vector<VertexDefinition> m_layout;
	std::vector<DrawInstruction> m_drawInstructions;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
private:
	Mesh( eMeshShape const &shape );

public:
	Mesh( );
	Mesh( eVertexType const & vertexType );
	Mesh( std::string const & textString, float scale = 12.f, BitmapFont const * font = nullptr );
	Mesh( MeshBuilder const *meshBuilder, eVertexType const & vertexType );
	~Mesh( );

	unsigned int GetVBOID( ) const { return m_vboID; }
	unsigned int GetIBOID( ) const { return m_iboID; }
	std::vector< VertexDefinition > const & GetLayout( ) const;
	std::vector< DrawInstruction > const & GetDrawInstructions( ) const;
	int GetVertexSize( ) const;
	void SetVertexLayout( eVertexType const &vertexType );
	//Returns the extents of the string mesh
	Vector3f Update( std::string const & newText, float scale = 12.f, BitmapFont const * font = nullptr );
	void Update( MeshBuilder const * meshBuilder );
};