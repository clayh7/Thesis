#include "Engine/UISystem/UISprite.hpp"

#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteResource.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UISprite::s_UISpriteRegistration( "UISprite", &UISprite::CreateWidgetFromXML );
STATIC char const * UISprite::DEFAULT_SPRITE_ID = "square";
STATIC char const * UISprite::PROPERTY_SPRITE_ID = "SpriteID";
STATIC char const * UISprite::PROPERTY_ROTATION_DEGREES = "RotationDegrees";


//-------------------------------------------------------------------------------------------------
UIWidget * UISprite::CreateWidgetFromXML( XMLNode const & node )
{
	return new UISprite( node );
}


//-------------------------------------------------------------------------------------------------
UISprite::UISprite( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_spriteData( nullptr )
{
	SetProperty( PROPERTY_SPRITE_ID, DEFAULT_SPRITE_ID );
	SetProperty( PROPERTY_ROTATION_DEGREES, 0.f );

	SetupRenderers( );
}


//-------------------------------------------------------------------------------------------------
UISprite::UISprite( XMLNode const & node )
	: UISprite( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UISprite::~UISprite( )
{
	delete m_quad;
	m_quad = nullptr;

	delete m_quadMesh;
	m_quadMesh = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UISprite::Update( )
{
	if( m_hidden )
	{
		return;
	}

	if( m_dirty )
	{
		UpdateRenderers( );
		m_dirty = false;
	}

	UIWidget::Update( );
}


//-------------------------------------------------------------------------------------------------
void UISprite::Render( ) const
{
	if( m_hidden )
	{
		return;
	}

	m_quad->Render( );
	UIWidget::Render( );
}


//-------------------------------------------------------------------------------------------------
void UISprite::SetupRenderers( )
{
	m_spriteData = g_SpriteRenderSystem->GetSpriteResource( DEFAULT_SPRITE_ID );

	//Dummy values to get them set up
	MeshBuilder quadBuilder;
	quadBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( -0.5f, -0.5f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ) );
	quadBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ), Vector3f( 0.5f, 0.5f, 0.f ) );

	Material * spriteMaterial = m_spriteData->GetMaterial( );
	m_quadMesh = new Mesh( &quadBuilder, eVertexType_PCU );
	m_quad = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	CreateSpriteMesh( m_quadMesh );
	m_quad->SetMeshAndMaterial( m_quadMesh, spriteMaterial );
	m_quad->SetUniform( "uTexDiffuse", m_spriteData->GetTexture( ) );
}


//-------------------------------------------------------------------------------------------------
void UISprite::UpdateRenderers( )
{
	//Update Sprite Quad
	std::string currentSpriteID;
	GetProperty( PROPERTY_SPRITE_ID, currentSpriteID );
	if( currentSpriteID != m_spriteData->m_id )
	{
		m_spriteData = g_SpriteRenderSystem->GetSpriteResource( currentSpriteID );
		m_quad->SetUniform( "uTexDiffuse", m_spriteData->GetTexture( ) );
	}

	//Update Mesh & Material
	CreateSpriteMesh( m_quadMesh );
	Material * spriteMaterial = m_spriteData->GetMaterial( );
	m_quad->SetMeshAndMaterial( m_quadMesh, spriteMaterial );

	//Update Quad Transform
	Transform quadTransform = CalcQuadTransform( );
	m_quad->SetTransform( quadTransform );
	m_quad->Update( );
}


//-------------------------------------------------------------------------------------------------
void UISprite::CreateSpriteMesh( Mesh * out_mesh )
{
	MeshBuilder spriteBuilder;
	AABB2f uvCoords = m_spriteData->GetUVBounds( );
	spriteBuilder.AddQuad( Vector2f( -0.5f ), Vector2f( 0.5f ), uvCoords );
	out_mesh->Update( &spriteBuilder );
}


//-------------------------------------------------------------------------------------------------
Transform UISprite::CalcQuadTransform( )
{
	Vector3f position = GetWorldPosition( eAnchor_CENTER );
	Vector3f scale = GetSize( );
	return Transform( position, Matrix4f::IDENTITY, scale );
}


//-------------------------------------------------------------------------------------------------
void UISprite::PopulateFromXML( XMLNode const & node )
{
	//Set properties from XML
	for( int childPropertyIndex = 0; childPropertyIndex < node.nChildNode( ); ++childPropertyIndex )
	{
		XMLNode childProperty = node.getChildNode( childPropertyIndex );
		if( strcmp( childProperty.getName( ), PROPERTY_SPRITE_ID ) == 0 )
		{
			std::string spriteID = ReadXMLAttribute( childProperty, STRING_VALUE, DEFAULT_SPRITE_ID );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_SPRITE_ID, spriteID, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_ROTATION_DEGREES ) == 0 )
		{
			float spriteRotation = ReadXMLAttribute( childProperty, STRING_VALUE, 0.f );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_ROTATION_DEGREES, spriteRotation, ParseState( state ) );
		}
	}
}