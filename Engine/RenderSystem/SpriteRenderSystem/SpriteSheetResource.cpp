#include "Engine/RenderSystem/SpriteRenderSystem/SpriteSheetResource.hpp"

#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
SpriteSheetResource::SpriteSheetResource( std::string const & id, std::string const & filename, Vector2i const & spriteSheetSize )
	: m_id( CreateNewCString( id ) )
	, m_filename( CreateNewCString( filename ) )
	, m_texture( Texture::CreateOrLoadTexture( filename ) )
	, m_size( spriteSheetSize )
	, m_material( new Material( "Data/Shaders/sprite.vert", "Data/Shaders/sprite.frag" ) )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
SpriteSheetResource::~SpriteSheetResource( )
{
	delete m_material;
	m_material = nullptr;

	delete m_filename;
	m_filename = nullptr;

	delete m_id;
	m_id = nullptr;
}


//-------------------------------------------------------------------------------------------------
Material * SpriteSheetResource::GetMaterial( ) const
{
	return m_material;
}


//-------------------------------------------------------------------------------------------------
Texture const * SpriteSheetResource::GetTexture( ) const
{
	return m_texture;
}


//-------------------------------------------------------------------------------------------------
//TexCoords TopLeft (0,0) BottomRight (1,1)
//Sprite AABB2 TopLeft (mins) BottomRight (maxs)
AABB2f SpriteSheetResource::GetTexCoordsForSpriteCoords( int spriteX, int spriteY ) const
{
	Vector2i textureSize = m_texture->GetTexelSize( );
	float widthPerSprite = (float) textureSize.x / (float) m_size.x;
	float heightPerSprite = (float) textureSize.y / (float) m_size.y;
	float texCoordX = spriteX * widthPerSprite;
	float texCoordY = spriteY * heightPerSprite;

	Vector2f mins = Vector2f( texCoordX / (float) textureSize.x, texCoordY / (float) textureSize.y );
	Vector2f maxs = Vector2f( ( texCoordX + widthPerSprite ) / (float) textureSize.x, ( texCoordY + heightPerSprite ) / (float) textureSize.y );
	return AABB2f( mins, maxs );
}


//-------------------------------------------------------------------------------------------------
AABB2f SpriteSheetResource::GetTexCoordsForSpriteIndex( int spriteIndex ) const
{
	int indexX = spriteIndex % m_size.x;
	int indexY = (int) floor( spriteIndex / m_size.x );
	return GetTexCoordsForSpriteCoords( indexX, indexY );
}