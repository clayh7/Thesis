#pragma once

#include <string>
#include <vector>
#include <map>
#include "Engine/RenderSystem/OpenGLExtensions.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/RenderSystem/RenderState.hpp"
#include "Engine/RenderSystem/VertexDefinition.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class Renderer;
class Texture;
class BitmapFont;
class Camera3D;
class MeshRenderer;
class Framebuffer;
class Mesh;
class Material;
class AABB3;
class Matrix4f;
class Vertex_PCU;
class Vertex_PC;
class VertexDefinition;
class Uniform;
typedef unsigned int GLenum;


//-------------------------------------------------------------------------------------------------
extern Renderer * g_RenderSystem;


//-------------------------------------------------------------------------------------------------
class Renderer
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * DEFAULT_FONT;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	Framebuffer * m_activeFBO;
	Camera3D const * m_activeCamera;
	RenderState m_currentRenderState;
	Color m_currentClearColor;
	unsigned int m_currentTextureID;
	float m_currentLineWidth;
	float m_currentPointSize;

public:
	Renderer( );
	~Renderer( );

	void Initialize( );
	void ClearScreen( const Color& clearColor );

	//-------------------------------------------------------------------------------------------------
	// SHADER DRAWING
	//-------------------------------------------------------------------------------------------------
	//#TODO: Should I move the Create methods into their respective classes?
	//#TODO: Add the ability to draw lines
	GLuint CreateSampler( GLenum const &min_filter, GLenum const &mag_filter, GLenum const &u_wrap, GLenum const &v_wrap );
	GLuint CreateRenderBuffer( void const *data, size_t count, size_t elem_size, GLenum const &usage = GL_STATIC_DRAW );
	void UpdateRenderBuffer( GLuint bufferID, void const *data, size_t count, size_t elem_size, GLenum const &usage = GL_STATIC_DRAW );
	void CreateOrUpdateRenderBuffer( GLuint * bufferID, void const *data, size_t count, size_t elem_size, GLenum const &usage = GL_STATIC_DRAW );
	GLuint CreateShader( std::string const &filename, GLenum shader_type );
	GLuint CreateAndLinkProgram( GLuint vs, GLuint fs, std::string const &debugFilepath );
	void BindMeshToVAO( GLuint vaoID, Mesh const *mesh, Material const *material );
	void BindShaderProgramProperty( int bindPoint, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset );
	void BindTextureSampler( GLuint samplerID, GLuint textureID, unsigned int textureIndexPort, int bindLocation );

	Camera3D const * GetActiveCamera( ) const { return m_activeCamera; }
	GLenum GetVertexDataType( VertexDataType const & type ) const;
	void SetActiveCamera( Camera3D const *viewCamera ) { m_activeCamera = viewCamera; }
	void SetShaderProgramUniforms( GLuint samplerID, std::map<std::string, Uniform*> const &meshRenderer );
	void SetMatrixUniforms( MeshRenderer* setMeshRenderer );
	void MeshRender( MeshRenderer const *meshRenderer );

	//-------------------------------------------------------------------------------------------------
	// FRAME BUFFERS
	//-------------------------------------------------------------------------------------------------
	void DeleteFramebuffer( Framebuffer *fbo );
	void BindFramebuffer( Framebuffer *fbo );
	void FramebufferCopyToBack( Framebuffer *fbo );
	
	//-------------------------------------------------------------------------------------------------
	// RENDER STATE FUNCTIONS
	//-------------------------------------------------------------------------------------------------
	void ApplyRenderState( RenderState const &renderState );
	GLenum GetOpenGLDrawMode( const ePrimitiveType& drawMode ) const;
	float GetLineWidth( ) const { return m_currentLineWidth; }
	float GetPointSize( ) const { return m_currentPointSize; }
	void SetCullFace( bool enable );
	void SetDepthWrite( bool enable );
	void SetDepthTest( bool enable );
	void SetBlending( const eBlending& setBlendingMode );
	void SetDrawMode( eDrawMode const &drawMode );
	void SetWindingOrder( bool clockwise );
	void SetLineWidth( float size );
	void SetPointSize( float size );
	void EnableAlphaTesting( float threshold = 0.5f );
	void DisableAlphaTesting( );

	//-------------------------------------------------------------------------------------------------
	// INFORMATION ABOUT GRAPHICS
	//-------------------------------------------------------------------------------------------------
	std::string GetOpenGLVersion( ) const;
	std::string GetGLSLVersion( ) const;
	BitmapFont const * GetDefaultFont( ) const;
	Framebuffer * GetActiveFBO( ) const;
};