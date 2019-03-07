#include "Engine/UISystem/UIContainer.hpp"

#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/Math/Vector2i.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UIContainer::s_UIContainerRegistration( "UIContainer", &UIContainer::CreateWidgetFromXML );
STATIC char const * UIContainer::PROPERTY_BORDER_COLOR = "BorderColor";
STATIC char const * UIContainer::PROPERTY_BORDER_SIZE = "BorderSize";
STATIC char const * UIContainer::PROPERTY_BACKGROUND_COLOR = "BackgroundColor";
STATIC char const * UIContainer::PROPERTY_GRID_SIZE = "GridSize";
STATIC char const * UIContainer::PROPERTY_ON_ADD_ITEM = "OnAddItem";


//-------------------------------------------------------------------------------------------------
UIWidget * UIContainer::CreateWidgetFromXML( XMLNode const & node )
{
	return new UIContainer( node );
}


//-------------------------------------------------------------------------------------------------
UIContainer::UIContainer( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_borderMesh( nullptr )
	, m_border( nullptr )
	, m_background( nullptr )
{
	SetProperty( PROPERTY_BORDER_COLOR, Color::BLACK );
	SetProperty( PROPERTY_BORDER_SIZE, 0.f );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::WHITE );
	SetProperty( PROPERTY_GRID_SIZE, Vector2i( 2, 2 ) );

	SetupRenderers( );
}


//-------------------------------------------------------------------------------------------------
UIContainer::UIContainer( XMLNode const & node )
	: UIContainer( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UIContainer::~UIContainer( )
{
	delete m_borderMesh;
	m_borderMesh = nullptr;

	delete m_border;
	m_border = nullptr;

	delete m_background;
	m_background = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIContainer::Update( )
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
void UIContainer::Render( ) const
{
	if( m_hidden )
	{
		return;
	}

	m_background->Render( );
	m_border->Render( );

	UIWidget::Render( );
}


//-------------------------------------------------------------------------------------------------
Vector2f UIContainer::GetItemSize( ) const
{
	Vector2i gridSize;
	GetProperty( PROPERTY_GRID_SIZE, gridSize );

	Vector2f size = GetSize( );
	size.x /= (float) gridSize.x;
	size.y /= (float) gridSize.y;
	return size;
}


//-------------------------------------------------------------------------------------------------
Vector2f UIContainer::GetPositionForItem( UIItem const * childItem ) const
{
	for( size_t childIndex = 0; childIndex < m_children.size( ); ++childIndex )
	{
		UIWidget * childWidget = (UIWidget*) childItem;
		if( m_children[childIndex] == childWidget )
		{
			Vector2f itemSize = GetItemSize( );
			Vector2i gridSize;
			GetProperty( PROPERTY_GRID_SIZE, gridSize );
			int numOfColumns = gridSize.x;
			int column = (int)childIndex % numOfColumns;
			int row = (int)childIndex / numOfColumns;
			Vector2f offsetToItemMiddle = itemSize / 2.f;
			Vector2f gridPosition = Vector2f( itemSize.x * (float)column, itemSize.y * (float)row );
			Vector2f position = GetWorldPosition( eAnchor_BOTTOM_LEFT );
			return position + gridPosition + offsetToItemMiddle;
		}
	}

	ERROR_AND_DIE( "This function is for Containers and their children" );
	//return Vector2f::ZERO;
}


//-------------------------------------------------------------------------------------------------
bool UIContainer::IsFull( ) const
{
	Vector2i gridSize = Vector2i::ZERO;
	GetProperty( PROPERTY_GRID_SIZE, gridSize );
	int maxItems = gridSize.x * gridSize.y;
	int childCount = (int)m_children.size( );
	return childCount >= maxItems;
}


//-------------------------------------------------------------------------------------------------
bool UIContainer::IsEmpty( ) const
{
	int childCount = (int) m_children.size( );
	return childCount == 0;
}


//-------------------------------------------------------------------------------------------------
void UIContainer::AddItem( UIItem * item )
{
	//Trigger Item added event
	std::string onAddItemEvent;
	GetProperty( PROPERTY_ON_ADD_ITEM, onAddItemEvent );
	NamedProperties eventParams;
	eventParams.Set( "item", item );
	AddChild( (UIWidget*) item );
	EventSystem::TriggerEvent( onAddItemEvent, eventParams );
}


//-------------------------------------------------------------------------------------------------
void UIContainer::SetupRenderers( )
{
	//Dummy values to get them set up
	MeshBuilder borderBuilder;
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( -0.5f, -0.5f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ) );
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ), Vector3f( 0.5f, 0.5f, 0.f ) );

	m_borderMesh = new Mesh( &borderBuilder, eVertexType_PCU );
	m_border = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_background = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
}


//-------------------------------------------------------------------------------------------------
void UIContainer::UpdateRenderers( )
{
	//Update Border
	CreateBorderMesh( m_borderMesh );

	Color borderColor;
	GetProperty( PROPERTY_BORDER_COLOR, borderColor );

	m_border->SetMesh( m_borderMesh );
	m_border->SetUniform( "uColor", borderColor );
	m_border->Update( );

	//Update Background
	Color backgroundColor;
	GetProperty( PROPERTY_BACKGROUND_COLOR, backgroundColor );

	Transform backgroundTransform = CalcBackgroundTransform( );

	m_background->SetUniform( "uColor", backgroundColor );
	m_background->SetTransform( backgroundTransform );
	m_background->Update( );
}


//-------------------------------------------------------------------------------------------------
void UIContainer::CreateBorderMesh( Mesh * out_mesh )
{
	MeshBuilder borderBuilder;
	float borderSize = 0.f;
	GetProperty( PROPERTY_BORDER_SIZE, borderSize );

	Vector2f topLeft = GetWorldPosition( eAnchor_TOP_LEFT ) + Vector2f( -borderSize, borderSize );
	Vector2f bottomRight = GetWorldPosition( eAnchor_TOP_RIGHT ) + Vector2f( borderSize, 0.f );
	borderBuilder.AddQuad( topLeft, bottomRight );

	topLeft = GetWorldPosition( eAnchor_BOTTOM_LEFT ) + Vector2f( -borderSize, 0.f );
	bottomRight = GetWorldPosition( eAnchor_BOTTOM_RIGHT ) + Vector2f( borderSize, -borderSize );
	borderBuilder.AddQuad( topLeft, bottomRight );

	topLeft = GetWorldPosition( eAnchor_TOP_LEFT ) + Vector2f( -borderSize, 0.f );
	bottomRight = GetWorldPosition( eAnchor_BOTTOM_LEFT ) + Vector2f( 0.f, 0.f );
	borderBuilder.AddQuad( topLeft, bottomRight );

	topLeft = GetWorldPosition( eAnchor_TOP_RIGHT ) + Vector2f( 0.f, 0.f );
	bottomRight = GetWorldPosition( eAnchor_BOTTOM_RIGHT ) + Vector2f( borderSize, 0.f );
	borderBuilder.AddQuad( topLeft, bottomRight );

	out_mesh->Update( &borderBuilder );
}


//-------------------------------------------------------------------------------------------------
Transform UIContainer::CalcBackgroundTransform( )
{
	Vector2f position = GetWorldPosition( eAnchor_CENTER );
	Vector2f scale = GetSize( );

	return Transform( position, Matrix4f( ), scale );
}


//-------------------------------------------------------------------------------------------------
void UIContainer::PopulateFromXML( XMLNode const & node )
{
	//Set properties from XML
	for( int childPropertyIndex = 0; childPropertyIndex < node.nChildNode( ); ++childPropertyIndex )
	{
		XMLNode childProperty = node.getChildNode( childPropertyIndex );
		//Border
		if( strcmp( childProperty.getName( ), PROPERTY_BORDER_COLOR ) == 0 )
		{
			Color borderColor = ReadXMLAttribute( childProperty, STRING_VALUE, Color::BLACK );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_BORDER_COLOR, borderColor, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_BORDER_SIZE ) == 0 )
		{
			float borderSize = ReadXMLAttribute( childProperty, STRING_VALUE, 0.f );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_BORDER_SIZE, borderSize, ParseState( state ) );
		}
		//Background
		else if( strcmp( childProperty.getName( ), PROPERTY_BACKGROUND_COLOR ) == 0 )
		{
			Color backgroundColor = ReadXMLAttribute( childProperty, STRING_VALUE, Color::WHITE );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_BACKGROUND_COLOR, backgroundColor, ParseState( state ) );
		}
		//Grid
		else if( strcmp( childProperty.getName( ), PROPERTY_GRID_SIZE ) == 0 )
		{
			Vector2i gridSize = ReadXMLAttribute( childProperty, STRING_VALUE, Vector2i::ONE );
			SetProperty( PROPERTY_GRID_SIZE, gridSize, eWidgetState_ALL );
		}
		//Event callbacks
		else if( strcmp( childProperty.getName( ), PROPERTY_ON_ADD_ITEM ) == 0 )
		{
			std::string eventName = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_ON_ADD_ITEM, eventName, ParseState( state ) );
		}
	}
}