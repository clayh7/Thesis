#include "Engine/RenderSystem/SpriteRenderSystem/SpriteResource.hpp"

#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteSheetResource.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
SpriteResource::SpriteResource( std::string const & id, std::string const & filename )
	: m_id( CreateNewCString( id ) )
	, m_filename( CreateNewCString( filename ) )
	, m_texture( Texture::CreateOrLoadTexture( filename ) )
	, m_material( new Material( "Data/Shaders/sprite.vert", "Data/Shaders/sprite.frag" ) )
	, m_spriteSheetResource( nullptr )
	, m_spriteSheetIndex( 0 )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
SpriteResource::SpriteResource( std::string const & id, SpriteSheetResource const * spriteSheetResource, int index )
	: m_id( CreateNewCString( id ) )
	, m_filename( nullptr )
	, m_texture( nullptr )
	, m_material( nullptr )
	, m_spriteSheetResource( spriteSheetResource )
	, m_spriteSheetIndex( index )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
SpriteResource::~SpriteResource( )
{
	//These may not have been set, may only be in SpriteSheetResources
	if( m_material )
	{
		delete m_material;
	}
	m_material = nullptr;

	if( m_filename )
	{
		delete m_filename;
	}
	m_filename = nullptr;

	delete m_id;
	m_id = nullptr;
}


//-------------------------------------------------------------------------------------------------
Material * SpriteResource::GetMaterial( ) const
{
	if( m_spriteSheetResource )
	{
		return m_spriteSheetResource->GetMaterial( );
	}
	else
	{
		return m_material;
	}
}


//-------------------------------------------------------------------------------------------------
Texture const * SpriteResource::GetTexture( ) const
{
	if( m_spriteSheetResource )
	{
		return m_spriteSheetResource->GetTexture( );
	}
	else
	{
		return m_texture;
	}
}


//-------------------------------------------------------------------------------------------------
AABB2f SpriteResource::GetUVBounds( ) const
{
	if( m_spriteSheetResource )
	{
		return m_spriteSheetResource->GetTexCoordsForSpriteIndex( m_spriteSheetIndex );
	}
	else
	{
		return AABB2f::ZERO_TO_ONE;
	}
}
