#pragma once

#include <string>
#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
class Material;
class Texture;
class AABB2f;


//-------------------------------------------------------------------------------------------------
class SpriteSheetResource
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	char const * m_id;
	char const * m_filename;
	Vector2i m_size;

private:
	Texture const * m_texture;
	Material * m_material;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	SpriteSheetResource( std::string const & id, std::string const & filename, Vector2i const & spriteSheetSize );
	SpriteSheetResource( SpriteSheetResource const & copy ) = delete;
	~SpriteSheetResource( );

	Material * GetMaterial( ) const;
	Texture const * GetTexture( ) const;
	AABB2f GetTexCoordsForSpriteCoords( int spriteX, int spriteY ) const;
	AABB2f GetTexCoordsForSpriteIndex( int spriteIndex ) const;
};