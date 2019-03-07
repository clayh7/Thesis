#pragma once
#include <string>


//-------------------------------------------------------------------------------------------------
class Texture;
class Material;
class SpriteSheetResource;
class AABB2f;


//-------------------------------------------------------------------------------------------------
class SpriteResource
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	char const * m_id;
	char const * m_filename;

private:
	Texture const * m_texture;
	Material * m_material;
	SpriteSheetResource const * m_spriteSheetResource;
	int m_spriteSheetIndex;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	SpriteResource( std::string const & id, std::string const & filename );
	SpriteResource( std::string const & id, SpriteSheetResource const * spriteSheetResource, int index );
	SpriteResource( SpriteResource const & copy ) = delete;
	~SpriteResource( );

	Material * GetMaterial( ) const;
	Texture const * GetTexture( ) const;
	AABB2f GetUVBounds( ) const;
};