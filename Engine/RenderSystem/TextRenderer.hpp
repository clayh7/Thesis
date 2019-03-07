#pragma once

#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/UISystem/UICommon.hpp"


//-------------------------------------------------------------------------------------------------
class BitmapFont;
class Mesh;
class Material;


//-------------------------------------------------------------------------------------------------
class TextRenderer : public MeshRenderer
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	std::string m_content;
	size_t m_contentHash;
	float m_size;
	Vector2f m_screenPosition;
	Vector3f m_extents;
	eAlignment m_alignment;
	BitmapFont const * m_font;

	Mesh * m_textMesh;
	Material * m_textMaterial;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	TextRenderer( std::string const & content, Vector2f const & screenPosition, float size = 12.f, BitmapFont const * font = nullptr );
	~TextRenderer( );

	void SetText( std::string const & content );
	void SetSize( float size );
	void SetFont( BitmapFont const * font );
	void SetPosition( Vector2f const & screenPosition );
	void SetColor( Color const & color );
	void SetAlignment( eAlignment const & alignment );

	std::string const & GetContent( ) const;

private:
	void SetPositionWithAlignment( );
};