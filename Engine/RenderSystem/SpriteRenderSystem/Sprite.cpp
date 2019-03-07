#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"

#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteLayer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteResource.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"


//-------------------------------------------------------------------------------------------------
Sprite::Sprite( std::string const & spriteID, int layer /*= 0*/, bool ignoreView /*= false*/ )
	: m_enabled( true )
	, m_layerID( layer )
	, m_spriteResource( nullptr )
	, m_renderer( g_SpriteRenderSystem )
	, m_spriteID( spriteID )
	, m_position( Vector2f::ZERO )
	, m_pivot( Vector2f( 0.5f, 0.5f ) )
	, m_rotationDegrees( 0.f )
	, m_scale( Vector2f::ONE )
	, m_color( Color::WHITE )
	, m_ignoreView( ignoreView )
	, m_nextSprite( nullptr )
	, m_prevSprite( nullptr )
{
	m_spriteResource = g_SpriteRenderSystem->GetSpriteResource( spriteID );
}


//-------------------------------------------------------------------------------------------------
Sprite::~Sprite( )
{
	SpriteLayer * layer = m_renderer->CreateOrGetLayer( m_layerID );
	layer->RemoveSprite( this );
}


//-------------------------------------------------------------------------------------------------
void Sprite::Render( ) const
{
	m_renderer->RenderSprite( this );
}


//-------------------------------------------------------------------------------------------------
void Sprite::ConstructMesh( Mesh * out_updateMesh, MeshBuilder & meshBuilder ) const
{
	meshBuilder.Clear( );

	Vector2f pivot = Vector2f( m_pivot.x * GetWidth( ), m_pivot.y * GetHeight( ) );
	Vector3f bottomLeftOffset = Vector3f( -pivot, 0.f );
	Vector3f bottomRightOffset = Vector3f( Vector2f( GetWidth( ) - pivot.x, -pivot.y ), 0.f );
	Vector3f topRightOffset = Vector3f( Vector2f( GetWidth( ) - pivot.x, GetHeight( ) - pivot.y ), 0.f );
	Vector3f topLeftOffset = Vector3f( Vector2f( -pivot.x, GetHeight( ) - pivot.y ), 0.f );

	Matrix4f scale;
	scale.SetScale( Vector3f( m_scale, 1.f ) );
	Matrix4f rotation;
	rotation.MakeView( Vector3f::ZERO, Euler( 0.f, 0.f, m_rotationDegrees ) );

	//p = S * R * T | position = Scale * Rotation * Translation
	bottomLeftOffset = ( ( Vector4f( bottomLeftOffset, 1.f ) * scale ) * rotation ).XYZ( );
	bottomRightOffset = ( ( Vector4f( bottomRightOffset, 1.f ) * scale ) * rotation ).XYZ( );
	topRightOffset = ( ( Vector4f( topRightOffset, 1.f ) * scale ) * rotation ).XYZ( );
	topLeftOffset = ( ( Vector4f( topLeftOffset, 1.f ) * scale ) * rotation ).XYZ( );

	Vector3f position = Vector3f( m_position, 0.f );
	Vector3f bl = position + bottomLeftOffset;
	Vector3f br = position + bottomRightOffset;
	Vector3f tr = position + topRightOffset;
	Vector3f tl = position + topLeftOffset;

	meshBuilder.Begin( );
	meshBuilder.SetColor( m_color );
	AABB2f uvBounds = m_spriteResource->GetUVBounds( );
	meshBuilder.SetUV0( uvBounds.mins );
	meshBuilder.AddVertex( tl );
	meshBuilder.SetUV0( uvBounds.maxs.x, uvBounds.mins.y );
	meshBuilder.AddVertex( tr );
	meshBuilder.SetUV0( uvBounds.mins.x, uvBounds.maxs.y );
	meshBuilder.AddVertex( bl );
	meshBuilder.SetUV0( uvBounds.maxs );
	meshBuilder.AddVertex( br );
	meshBuilder.AddQuadIndicies( 0, 1, 2, 3 );
	meshBuilder.End( );

	out_updateMesh->Update( &meshBuilder );
}


//-------------------------------------------------------------------------------------------------
Material * Sprite::GetMaterial( ) const
{
	return m_spriteResource->GetMaterial( );
}


//-------------------------------------------------------------------------------------------------
Texture const * Sprite::GetTexture( ) const
{
	return m_spriteResource->GetTexture( );
}


//-------------------------------------------------------------------------------------------------
float Sprite::GetWidth( ) const
{
	float actualWidth = (float) m_spriteResource->GetTexture( )->m_texelSize.x;
	float importSize = m_renderer->GetImportSize( );
	float virtualSize = m_renderer->GetVirtualSize( );
	float convertedWidth = actualWidth / importSize * virtualSize;
	return convertedWidth;
}


//-------------------------------------------------------------------------------------------------
float Sprite::GetHeight( ) const
{
	float actualHeight = (float) m_spriteResource->GetTexture( )->m_texelSize.y;
	float importSize = m_renderer->GetImportSize( );
	float virtualSize = m_renderer->GetVirtualSize( );
	float convertedHeight = actualHeight / importSize * virtualSize;
	return convertedHeight;
}


//-------------------------------------------------------------------------------------------------
Vector2f Sprite::GetPivot( ) const
{
	return m_pivot;
}


//-------------------------------------------------------------------------------------------------
Vector2f Sprite::GetScale( ) const
{
	return m_scale;
}


//-------------------------------------------------------------------------------------------------
float Sprite::GetRadiusScaled( ) const
{
	float w = GetWidth( ) * GetScale( ).x;
	float h = GetHeight( ) * GetScale( ).y;
	return sqrt( w*w + h*h );
}


//-------------------------------------------------------------------------------------------------
std::string Sprite::GetID( ) const
{
	return m_spriteResource->m_id;
}


//-------------------------------------------------------------------------------------------------
bool Sprite::IsEnabled( ) const
{
	return m_enabled;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetEnabled( bool isEnabled )
{
	m_enabled = isEnabled;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetScale( Vector2f const & scale )
{
	m_scale = scale;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetColor( Color const & color )
{
	m_color = color;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetPivot( Vector2f const & pivot )
{
	m_pivot = pivot;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetPosition( Vector2f const & position )
{
	m_position = position;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetRotation( float degrees )
{
	m_rotationDegrees = degrees;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetRenderer( SpriteGameRenderer const * renderer )
{
	m_renderer = renderer;
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetID( std::string const & spriteID )
{
	if( m_spriteID != spriteID )
	{
		m_spriteResource = g_SpriteRenderSystem->GetSpriteResource( spriteID );
		m_spriteID = spriteID;
	}
}


//-------------------------------------------------------------------------------------------------
void Sprite::SetLayer( int layerID )
{
	//Don't need to set the layer if it's already that layer
	if( layerID == m_layerID )
	{
		return;
	}

	//Remove from previous layer
	SpriteLayer * currentLayer = g_SpriteRenderSystem->CreateOrGetLayer( m_layerID );
	currentLayer->RemoveSprite( this );

	//Add to next layer
	SpriteLayer * newLayer = g_SpriteRenderSystem->CreateOrGetLayer( layerID );
	newLayer->AddSprite( this );

	m_layerID = layerID;
}