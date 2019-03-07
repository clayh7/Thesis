#include "Engine/UISystem/UIItem.hpp"

#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteResource.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/UISystem/UIContainer.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UIItem::s_UIItemRegistration( "UIItem", &UIItem::CreateWidgetFromXML );

STATIC char const * UIItem::DEFAULT_SPRIE_ID = "square";
STATIC char const * UIItem::EVENT_PICK_UP_ITEM = "OnPickupItem";
STATIC char const * UIItem::EVENT_DROP_ITEM = "OnDropItem";
STATIC char const * UIItem::PROPERTY_SPRITE_ID = "SpriteID";


//-------------------------------------------------------------------------------------------------
UIWidget * UIItem::CreateWidgetFromXML( XMLNode const & node )
{
	return new UIItem( node );
}


//-------------------------------------------------------------------------------------------------
UIItem::UIItem( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_spriteData( nullptr )
{
	SetProperty( PROPERTY_SPRITE_ID, DEFAULT_SPRIE_ID );

	SetProperty( PROPERTY_ON_PRESSED, EVENT_PICK_UP_ITEM );
	SetProperty( PROPERTY_ON_RELEASED, EVENT_DROP_ITEM );

	SetupRenderers( );

	EventSystem::RegisterEvent( EVENT_PICK_UP_ITEM, this, &UIItem::OnItemPickup );
	EventSystem::RegisterEvent( EVENT_DROP_ITEM, this, &UIItem::OnItemDrop );
}


//-------------------------------------------------------------------------------------------------
UIItem::UIItem( XMLNode const & node )
	: UIItem( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UIItem::~UIItem( )
{
	EventSystem::Unregister( this );

	delete m_quad;
	m_quad = nullptr;

	delete m_quadMesh;
	m_quadMesh = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIItem::Update( )
{
	if( m_hidden )
	{
		return;
	}

	if( m_dirty || IsHeld( ) )
	{
		UpdateRenderers( );
		m_dirty = false;
	}

	UIWidget::Update( );
}


//-------------------------------------------------------------------------------------------------
void UIItem::Render( ) const
{
	//#TODO: Bug, it'll get draw twice if it's currently held, because we need to draw it again after the UISystem so it shows on top of everything else...
	if( m_hidden )
	{
		return;
	}

	m_quad->Render( );
	UIWidget::Render( );
}


//-------------------------------------------------------------------------------------------------
Vector2f UIItem::GetSize( ) const
{
	UIContainer * parentContainer = dynamic_cast<UIContainer*>( m_parent );

	//Currently held
	if( IsHeld( ) )
	{
		return UIWidget::GetSize( );
	}

	//our parent is a container
	else if( parentContainer )
	{
		return parentContainer->GetItemSize( );
	}

	//Normal widget mode
	else
	{
		return UIWidget::GetSize( );
	}
}


//-------------------------------------------------------------------------------------------------
void UIItem::SetupRenderers( )
{
	m_spriteData = g_SpriteRenderSystem->GetSpriteResource( DEFAULT_SPRIE_ID );

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
void UIItem::UpdateRenderers( )
{
	//Update Sprite ID
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
	Transform quadTransform = CalcItemTransform( );
	m_quad->SetTransform( quadTransform );
	m_quad->Update( );
}


//-------------------------------------------------------------------------------------------------
void UIItem::CreateSpriteMesh( Mesh * out_mesh )
{
	MeshBuilder spriteBuilder;
	AABB2f uvCoords = m_spriteData->GetUVBounds( );
	spriteBuilder.AddQuad( Vector2f( -0.5f ), Vector2f( 0.5f ), uvCoords );
	out_mesh->Update( &spriteBuilder );
}


//-------------------------------------------------------------------------------------------------
Transform UIItem::CalcItemTransform( )
{
	Vector3f position = GetWorldPosition( eAnchor_CENTER );
	Vector3f scale = GetSize( );
	return Transform( position, Matrix4f::IDENTITY, scale );
}


//-------------------------------------------------------------------------------------------------
void UIItem::PopulateFromXML( XMLNode const & node )
{
	//Set properties from XML
	for( int childPropertyIndex = 0; childPropertyIndex < node.nChildNode( ); ++childPropertyIndex )
	{
		XMLNode childProperty = node.getChildNode( childPropertyIndex );
		if( strcmp( childProperty.getName( ), PROPERTY_SPRITE_ID ) == 0 )
		{
			std::string spriteID = ReadXMLAttribute( childProperty, STRING_VALUE, DEFAULT_SPRIE_ID );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_SPRITE_ID, spriteID, ParseState( state ) );
		}
	}
}


//-------------------------------------------------------------------------------------------------
Vector2f UIItem::GetWorldPosition( eAnchor const & anchor, UIWidget const * forChild /*= nullptr */ ) const
{
	eAnchor currentAnchor = GetAnchor( );
	Vector2f currentOffset = GetOffset( );
	Vector2f currentWorldPosition = GetLocalPosition( anchor ) - GetLocalPosition( currentAnchor ) + currentOffset;
	UIContainer * parentContainer = dynamic_cast<UIContainer*>( m_parent );

	//Currently held
	if( IsHeld( ) )
	{
		Vector2f cursorPosition = UISystem::GetCursorUIPosition( );
		currentWorldPosition += cursorPosition;
	}

	//our parent is a container
	else if( parentContainer )
	{
		Vector2f centerOfItem = parentContainer->GetPositionForItem( this );
		currentWorldPosition += centerOfItem;
	}

	//Normal widget mode
	else
	{
		return UIWidget::GetWorldPosition( anchor, forChild );
	}

	return currentWorldPosition;
}


//-------------------------------------------------------------------------------------------------
bool UIItem::IsHeld( ) const
{
	UIItem * heldItem = g_UISystem->GetHeldItem( );
	return heldItem == this;
}


//-------------------------------------------------------------------------------------------------
void UIItem::OnItemPickup( NamedProperties & )
{
	if( GetState( ) == eWidgetState_HIGHLIGHTED )
	{
		g_UISystem->SetHeldItem( this );
	}
}


//-------------------------------------------------------------------------------------------------
void UIItem::OnItemDrop( NamedProperties & )
{
	//Only drop this item if it was the one being held
	if( !IsHeld( ) )
	{
		return;
	}

	//Important to set holding to null first, else the UISystem will think the cursor is current inside of the held item
	g_UISystem->SetHeldItem( nullptr );

	UIWidget * selectedWidget = g_UISystem->GetWidgetUnderCursor( );
	UIContainer * selectedContainer = dynamic_cast<UIContainer*>( selectedWidget );
	UIContainer * parentContainer = dynamic_cast<UIContainer*>( m_parent );
	
	//If you dropped the item over a container and you were previously in a container
	if( selectedContainer && parentContainer )
	{
		//If there is still room in the selected container
		if( !selectedContainer->IsFull( ) )
		{
			//This also removes this from its current parent
			selectedContainer->AddItem( this );
			return;
		}
	}

	//Alternatively, if you drop this item on an item, add it to that item's container
	UIItem * selectedItem = dynamic_cast<UIItem*>( selectedWidget );
	if( selectedItem )
	{
		selectedContainer = dynamic_cast<UIContainer*>( selectedItem->m_parent );
		if( selectedContainer && parentContainer )
		{
			if( !selectedContainer->IsFull( ) )
			{
				selectedContainer->AddItem( this );
				return;
			}
		}
	}
}