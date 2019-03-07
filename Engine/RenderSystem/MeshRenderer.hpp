#pragma once

#include <map>
#include "Engine/Math/Transform.hpp"
#include "Engine/RenderSystem/Mesh.hpp" //Need MeshShape enum
#include "Engine/RenderSystem/RenderState.hpp"


//-------------------------------------------------------------------------------------------------
class Color;
class Material;
class Texture;
class Matrix4f;
class Uniform;
class Attribute;
class Light;


//-------------------------------------------------------------------------------------------------
class MeshRenderer
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
protected:
	Transform m_transform;
	RenderState m_renderState;
	unsigned int m_vaoID;
	Mesh const * m_mesh;
	Material const * m_material;
	//#TODO: Replace all uniforms with NamedProperties (here and in Materials)
	std::map<std::string, Uniform*> m_uniforms; //Uniforms individual to this MeshRenderer. Overwrites Material

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	//#TODO: Be careful with this implementation of MeshRenderer, catch the cases where VAO = NULL
	MeshRenderer( Transform const &transform = Transform( ), RenderState const &renderState = RenderState::BASIC_3D );
	
	MeshRenderer( eMeshShape const &meshShape, Transform const &transform = Transform( ), RenderState const &renderState = RenderState::BASIC_3D );
	MeshRenderer( Mesh const *mesh, Transform const &transform = Transform( ), RenderState const &renderState = RenderState::BASIC_3D );
	MeshRenderer( Mesh const *mesh, Material const *material, Transform const &transform = Transform(), RenderState const &renderState = RenderState::BASIC_3D );
	~MeshRenderer( );

	void Update( bool onScreen = false );
	void Render( ) const;

	void CreateVAO( );

	RenderState GetRenderState( ) const;
	Matrix4f GetModelMatrix( ) const;
	Matrix4f GetViewMatrix( ) const;
	Matrix4f GetProjectionMatrix( ) const;
	unsigned int GetGPUProgramID( ) const;
	unsigned int GetSamplerID( ) const;
	unsigned int GetVAOID( ) const;
	unsigned int GetIBOID( ) const;
	std::vector<DrawInstruction> const & GetDrawInstructions( ) const;
	std::map<std::string, Uniform*> const & GetUniformList( ) const;
	std::map<std::string, Uniform*> const & GetMaterialUniformList( ) const;
	std::map<size_t, Attribute*> const & GetMaterialAttributeList( ) const;
	Vector3f GetPosition( );

	void SetLineWidth( float lineWidth );
	void SetPosition( Vector3f const & pos );
	void SetPosition( float xPos, float yPos, float zPos );
	void SetTransform( Transform const &transform );
	void SetMesh( Mesh const * mesh );
	void SetMaterial( Material const * setMaterial );
	void SetMeshAndMaterial( Mesh const * mesh, Material const * material );
	void RebindMeshAndMaterialToVAO( );//#TODO: Maybe make a force rebind
	void UpdateUniformBindpoints( );

	void SetUniform( std::string const & uniformName, uint32_t uniformValue );
	void SetUniform( std::string const & uniformName, int uniformValue );
	void SetUniform( std::string const & uniformName, float uniformValue );
	void SetUniform( std::string const & uniformName, Vector2f const & uniformValue );
	void SetUniform( std::string const & uniformName, Vector3f const & uniformValue );
	void SetUniform( std::string const & uniformName, Vector4f const & uniformValue );
	void SetUniform( std::string const & uniformName, Vector4i const & uniformValue );
	void SetUniform( std::string const & uniformName, Matrix4f const & uniformValue );
	void SetUniform( std::string const & uniformName, Color const & uniformValue );
	void SetUniform( std::string const & uniformName, std::string const & uniformValue );
	void SetUniform( std::string const & uniformName, Texture const * uniformValue );
										 
	void SetUniform( std::string const & uniformName, int * uniformValue );
	void SetUniform( std::string const & uniformName, float * uniformValue );
	void SetUniform( std::string const & uniformName, Vector2f * uniformValue );
	void SetUniform( std::string const & uniformName, Vector3f * uniformValue );
	void SetUniform( std::string const & uniformName, Vector4f * uniformValue );
	void SetUniform( std::string const & uniformName, Matrix4f * uniformValue );

	void SetUniform( std::vector<Light> const & uniformLights, int lightCount );
};