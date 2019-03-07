#pragma once

#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
class AABB2f;


//-------------------------------------------------------------------------------------------------
class Glyph
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	int m_id;
	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_xoffset;
	int m_yoffset;
	int m_xadvance;
	Vector2i m_textureSize;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Glyph( int id, int x, int y, int width, int height, int xoffset, int yoffset, int xadvance, Vector2i const & textureSize );
	~Glyph( );
	Vector2i GetOffset( ) const;
	Vector2i GetSize( ) const;
	AABB2f GetTexCoords( ) const;
};