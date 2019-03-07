#include "Engine/RenderSystem/TextRenderer.hpp"

#include "Engine/Core/Engine.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/RenderSystem/BitmapFont.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/Renderer.hpp"


//-------------------------------------------------------------------------------------------------
TextRenderer::TextRenderer( std::string const & content, Vector2f const & screenPosition, float size /*= 12.f */, BitmapFont const * font /*= nullptr*/ )
	: MeshRenderer( Transform( screenPosition ), RenderState::BASIC_2D )
	, m_content( content )
	, m_contentHash( std::hash<std::string>{}( content ) )
	, m_size( size )
	, m_screenPosition( screenPosition )
	, m_extents( 0.f )
	, m_font( font )
	, m_alignment( eAlignment_NONE )
	, m_textMesh( nullptr )
	, m_textMaterial( nullptr )
{
	if( !m_font )
	{
		m_font = g_RenderSystem->GetDefaultFont( );
	}
	m_textMesh = new Mesh( "", 12.f, m_font );
	m_extents = m_textMesh->Update( m_content, m_size, m_font );
	m_mesh = m_textMesh;
	m_textMaterial = Material::FONT_MATERIAL;
	m_material = m_textMaterial;
	
	SetUniform( "uFontTex", m_font->GetTextureID() );
	SetUniform( "uColor", Color::WHITE );
	SetAlignment( eAlignment_LEFT );

	g_RenderSystem->BindMeshToVAO( m_vaoID, m_mesh, m_material );

	//#TODO: Make the matrix proj and view be what they're supposed to be
	//Set Matricies
	Matrix4f view = Matrix4f::IDENTITY; //Matrix4f::View( );
	Matrix4f proj = Matrix4f::IDENTITY; //Orthonormal( 1600.f, 900.f );
	SetUniform( "uModel", GetModelMatrix( ) );
	SetUniform( "uView", view );
	SetUniform( "uProj", proj );
}


//-------------------------------------------------------------------------------------------------
TextRenderer::~TextRenderer( )
{
	delete m_textMesh;
	m_textMesh = nullptr;
}


//-------------------------------------------------------------------------------------------------
void TextRenderer::SetText( std::string const & content )
{
	//Rebuilding Mesh is expensive, only update if new
	size_t contentHash = std::hash<std::string>{ }( content );
	if( m_contentHash != contentHash )
	{
		m_content = content;
		m_contentHash = contentHash;
		m_extents = m_textMesh->Update( content, m_size, m_font );
		SetPositionWithAlignment( );
	}
}


//-------------------------------------------------------------------------------------------------
void TextRenderer::SetSize( float size )
{
	//Rebuilding Mesh is expensive, only update if new
	if( m_size != size )
	{
		m_size = size;
		m_extents = m_textMesh->Update( m_content, m_size, m_font );
	}
}


//-------------------------------------------------------------------------------------------------
void TextRenderer::SetFont( BitmapFont const * font )
{
	//Rebuilding Mesh is expensive, only update if new
	if( m_font != font )
	{
		m_font = font;
		m_extents = m_textMesh->Update( m_content, m_size, m_font );
	}
}


//-------------------------------------------------------------------------------------------------
void TextRenderer::SetPosition( Vector2f const & screenPosition )
{
	//Rebuilding Mesh is expensive, only update if new
	if( !( m_screenPosition == screenPosition ) )
	{
		m_screenPosition = screenPosition;
		SetPositionWithAlignment( );
	}
}


//-------------------------------------------------------------------------------------------------
void TextRenderer::SetColor( Color const & color )
{
	SetUniform( "uColor", color );
}


//-------------------------------------------------------------------------------------------------
void TextRenderer::SetAlignment( eAlignment const & alignment )
{
	if( m_alignment != alignment )
	{
		m_alignment = alignment;
		SetPositionWithAlignment( );
	}
}


//-------------------------------------------------------------------------------------------------
std::string const & TextRenderer::GetContent( ) const
{
	return m_content;
}


//-------------------------------------------------------------------------------------------------
void TextRenderer::SetPositionWithAlignment( )
{
	//Vector3f newPosition = Engine::ScreenToClipSpace( m_screenPosition );
	Vector3f newPosition = m_screenPosition;
	switch( m_alignment )
	{
	case eAlignment_LEFT:
		newPosition += Vector3f( 0.f, m_extents.y / 2.f, 0.f );
		break;
	case eAlignment_CENTER:
		newPosition += Vector3f( -m_extents.x / 2.f, m_extents.y / 2.f, 0.f );
		break;
	case eAlignment_RIGHT:
		newPosition += Vector3f( -m_extents.x, m_extents.y / 2.f, 0.f );
		break;
	}
	m_transform = Transform( newPosition );
	SetUniform( "uModel", GetModelMatrix( ) );
}