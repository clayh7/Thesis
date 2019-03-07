#include "Engine/RenderSystem/Glyph.hpp"

#include "Engine/Math/Vector2i.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/AABB2f.hpp"


//-------------------------------------------------------------------------------------------------
Glyph::Glyph( int id, int x, int y, int width, int height, int xoffset, int yoffset, int xadvance, Vector2i const & textureSize )
	: m_id( id )
	, m_x( x )
	, m_y( y )
	, m_width( width )
	, m_height( height )
	, m_xoffset( xoffset )
	, m_yoffset( yoffset )
	, m_xadvance( xadvance )
	, m_textureSize( textureSize )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Glyph::~Glyph( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Vector2i Glyph::GetOffset( ) const
{
	return Vector2i( m_xoffset, m_yoffset );
}


//-------------------------------------------------------------------------------------------------
Vector2i Glyph::GetSize( ) const
{
	return Vector2i( m_width, m_height );
}


//-------------------------------------------------------------------------------------------------
AABB2f Glyph::GetTexCoords( ) const
{
	float uvX = ( float ) m_x / ( float ) m_textureSize.x;
	float uvY = ( ( float ) m_y / ( float ) m_textureSize.y );
	Vector2f topLeft = Vector2f( uvX, uvY );
	float offsetX = ( float ) m_width / ( float ) m_textureSize.x;
	float offsetY = ( float ) m_height / ( float ) m_textureSize.y;
	Vector2f bottomRight = topLeft + Vector2f( offsetX, offsetY );

	return AABB2f( topLeft, bottomRight );
}
