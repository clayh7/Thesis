#include "Engine/UISystem/UISystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/UISystem/UIWidget.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/Camera3D.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * UISystem::DEFAULT_NAME = "";
STATIC char const * UISystem::UI_SKIN = "UISkin";
STATIC std::map<size_t, WidgetCreationFunc*> * UISystem::s_registeredWidgets = nullptr;
UISystem * g_UISystem = nullptr;


//-------------------------------------------------------------------------------------------------
UIWidgetRegistration::UIWidgetRegistration( std::string const & widgetName, WidgetCreationFunc * creationFunc )
{
	UISystem::RegisterWidget( widgetName, creationFunc );
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2f UISystem::ClipToUISystemPosition( Vector3f const & clipVector )
{
	float x = RangeMap( -1.f, 1.f, clipVector.x, 0.f, (float) VIRTUAL_WIDTH );
	float y = RangeMap( -1.f, 1.f, clipVector.y, 0.f, (float) VIRTUAL_HEIGHT );
	return Vector2f( x, y );
}


//-------------------------------------------------------------------------------------------------
void UISystem::RegisterWidget( std::string const & widgetName, WidgetCreationFunc * creationFunc )
{
	//Create it if it does not exist
	if( !s_registeredWidgets )
	{
		s_registeredWidgets = new std::map<size_t, WidgetCreationFunc *>( );
	}

	size_t widgetNameHash = std::hash<std::string>{ }( widgetName );
	s_registeredWidgets->insert( std::pair<size_t, WidgetCreationFunc*>( widgetNameHash, creationFunc ) );
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::CreateWidgetFromName( std::string const & name, XMLNode const & data )
{
	size_t widgetNameHash = std::hash<std::string>{ }( name );
	auto foundRegisteredWidget = s_registeredWidgets->find( widgetNameHash );
	if( foundRegisteredWidget != s_registeredWidgets->end( ) )
	{
		return ( foundRegisteredWidget->second )( data );
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
Vector2f UISystem::GetCursorUIPosition( )
{
	Vector2f cursorPosition = g_InputSystem->GetMousePosition( true );
	Vector3f cursorClipPosition;// = ScreenToClipPosition( cursorPosition );
	Vector2i windowDimensions = GetWindowDimensions( );
	float x = RangeMap( 0.f, (float) windowDimensions.x, cursorPosition.x, -1.f, 1.f );
	float y = RangeMap( 0.f, (float) windowDimensions.y, cursorPosition.y, -1.f, 1.f );
	cursorClipPosition = Vector3f( x, y, 0.f );
	//Matrix4f projectionMatrix = g_RenderSystem->GetActiveCamera( )->GetProjectionMatrix( );
	//invert projMat
	//multiply
	return ClipToUISystemPosition( cursorClipPosition );
}


//-------------------------------------------------------------------------------------------------
//My compiler does not compile these classes, and therefore they are not being added
//So I will explicitly use them here, so that they will all forcibly be compiled
#include "Engine/UISystem/UIButton.hpp"
#include "Engine/UISystem/UIBox.hpp"
#include "Engine/UISystem/UIContainer.hpp"
#include "Engine/UISystem/UIItem.hpp"
#include "Engine/UISystem/UILabel.hpp"
#include "Engine/UISystem/UIProgressBar.hpp"
#include "Engine/UISystem/UISprite.hpp"
#include "Engine/UISystem/UITextField.hpp"
void WakeUpUISystem( )
{
	static bool doOnce = true;
	if( doOnce )
	{
		UIWidget( );
		UIButton( );
		UIBox( );
		UIContainer( );
		UIItem( );
		UILabel( );
		UIProgressBar( );
		UISprite( );
		UITextField( );
		doOnce = false;
	}
}


//-------------------------------------------------------------------------------------------------
UISystem::UISystem( )
	: m_root( new UIWidget( ) )
	, m_highlightedWidget( nullptr )
	, m_selectedWidget( nullptr )
	, m_uiSpriteRenderer( new SpriteGameRenderer( ) )
	, m_heldItem( nullptr )
{
	WakeUpUISystem( );
	UpdateUISpriteRenderer( );

	//#TODO: Set root to have size of the virtual dimensions
	//#TODO: Maybe also set the aspect ratio some how?
	Vector2i windowDimensions = GetWindowDimensions( );
	m_root->SetProperty( UIWidget::PROPERTY_WIDTH, (float) VIRTUAL_WIDTH );
	m_root->SetProperty( UIWidget::PROPERTY_HEIGHT, (float) VIRTUAL_HEIGHT );
}


//-------------------------------------------------------------------------------------------------
UISystem::~UISystem( )
{
	delete m_root;
	m_root = nullptr;

	delete s_registeredWidgets;
	s_registeredWidgets = nullptr;

	delete m_uiSpriteRenderer;
	m_uiSpriteRenderer = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UISystem::UpdateUISpriteRenderer( )
{
	Vector2i windowDimensions = GetWindowDimensions( );
	m_uiSpriteRenderer->SetAspectRatio( (float) windowDimensions.x, (float) windowDimensions.y );
	//m_uiSpriteRenderer->SetImportSize( 220000.f ); //256
	m_uiSpriteRenderer->SetImportSize( 900.f );
	m_uiSpriteRenderer->SetVirtualSize( (float) windowDimensions.y );
	m_uiSpriteRenderer->SetClearColor( Color::WHITE );
	Matrix4f view;
	view.MakeView( -Vector3f( (float) windowDimensions.x/2.f, (float) windowDimensions.y/2.f, 0.f ) );
	m_uiSpriteRenderer->SetView( view );
}


//-------------------------------------------------------------------------------------------------
void UISystem::Update( )
{
	//Update root dimensions to match screen
	//Only update if they've changed so we don't dirty everything every frame
	Vector2f windowDimensions = (Vector2f) GetWindowDimensions( );
	Vector2f rootSize = m_root->GetSize( );
	if( rootSize != windowDimensions )
	{
		//UpdateUISpriteRenderer( );
		//m_root->SetProperty( UIWidget::PROPERTY_WIDTH, windowDimensions.x );
		//m_root->SetProperty( UIWidget::PROPERTY_HEIGHT, windowDimensions.y );
	}
	
	//Update entire UI tree
	m_root->Update( );

	//No highlighting widgets if you're holding an item
	if( !IsHoldingAnItem( ) )
	{
		//Highlight widget
		UIWidget * currentHighlightedWidget = GetWidgetUnderCursor( );
		if( m_highlightedWidget != currentHighlightedWidget )
		{
			if( m_highlightedWidget )
			{
				m_highlightedWidget->Unhighlight( );
			}
			m_highlightedWidget = currentHighlightedWidget;
			if( m_highlightedWidget )
			{
				m_highlightedWidget->Highlight( );
			}
		}
	}

	//Pressed widget
	if( g_InputSystem->WasKeyJustPressed( eMouseButton_LEFT ) )
	{
		if( m_highlightedWidget )
		{
			m_highlightedWidget->Press( );
			Select( m_highlightedWidget );
		}
	}

	//Released widget
	if( g_InputSystem->WasKeyJustReleased( eMouseButton_LEFT ) )
	{
		if( m_highlightedWidget )
		{
			m_highlightedWidget->Release( );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::Render( ) const
{
	m_root->Render( );
	if( m_heldItem )
	{
		m_heldItem->Render( );
	}
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::CreateWidget( )
{
	UIWidget * widget = new UIWidget( );
	m_root->AddChild( widget );
	return widget;
}


//-------------------------------------------------------------------------------------------------
void UISystem::RemoveWidget( UIWidget * widget )
{
	//Sorry, you can't remove the root
	if( widget == m_root )
	{
		return;
	}

	UIWidget * parent = widget->GetParent( );
	parent->RemoveChild( widget );
	if( widget == m_highlightedWidget )
	{
		m_highlightedWidget = nullptr;
	}
	if( widget == m_selectedWidget )
	{
		m_selectedWidget = nullptr;
	}
	if( widget == m_heldItem )
	{
		m_heldItem = nullptr;
	}
	delete widget;
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::GetHighlightedWidget( ) const
{
	return m_highlightedWidget;
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::GetSelectedWidget( ) const
{
	return m_selectedWidget;
}


//-------------------------------------------------------------------------------------------------
UIItem * UISystem::GetHeldItem( ) const
{
	return m_heldItem;
}


//-------------------------------------------------------------------------------------------------
bool UISystem::IsHoldingAnItem( ) const
{
	return GetHeldItem( ) != nullptr;
}


//-------------------------------------------------------------------------------------------------
SpriteGameRenderer const * UISystem::GetRenderer( ) const
{
	return m_uiSpriteRenderer;
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::GetWidgetUnderCursor( ) const
{
	Vector2f cursorUIPosition = GetCursorUIPosition( );
	UIWidget * widget = m_root->FindWidgetUnderPosition( cursorUIPosition );
	if( widget == nullptr )
	{
		return m_root;
	}
	else
	{
		return widget;
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::SetHeldItem( UIItem * item )
{
	m_heldItem = item;
}


//-------------------------------------------------------------------------------------------------
void UISystem::Select( UIWidget * selectedWidget )
{
	//UIWidget * lastSelected = m_selectedWidget;
	//m_selectedWidget = selectedWidget;
	if( m_selectedWidget )
	{
		m_selectedWidget->Dirty( );
	}
	m_selectedWidget = selectedWidget;
}


//-------------------------------------------------------------------------------------------------
void UISystem::LoadUIFromXML( )
{
	std::vector<std::string> uiFiles = EnumerateFilesInFolder( "Data/UI/", "*.UI.xml" );

	//First: Load skins
	for( std::string const & uiFile : uiFiles )
	{
		//Have to get the first child to get into the XML structure
		XMLNode uiLayoutXML = XMLNode::openFileHelper( uiFile.c_str( ) ).getChildNode( 0 );
		int skinNodeCount = uiLayoutXML.nChildNode( UI_SKIN );
		for( int skinIndex = 0; skinIndex < skinNodeCount; ++skinIndex )
		{
			XMLNode skinNode = uiLayoutXML.getChildNode( UI_SKIN, skinIndex );
			AddSkinFromXML( skinNode );
		}
	}

	//Second: Load widgets
	for( std::string const & uiFile : uiFiles )
	{
		//Have to get the first child to get into the XML structure
		XMLNode uiLayoutXML = XMLNode::openFileHelper( uiFile.c_str( ) ).getChildNode( 0 );
		int childNodeCount = uiLayoutXML.nChildNode( );
		for( int childIndex = 0; childIndex < childNodeCount; ++childIndex )
		{
			XMLNode widgetNode = uiLayoutXML.getChildNode( childIndex );
			AddWidgetFromXML( widgetNode );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::AddSkinFromXML( XMLNode const & node )
{
	//Hash the name and store it in our skin map
	std::string skinName = ReadXMLAttribute( node, "name", "" );
	size_t skinHash = std::hash<std::string>{ }( skinName );
	
	//Add to skin dictionary
	m_skins.insert( std::pair<size_t, XMLNode>( skinHash, node ) );
}


//-------------------------------------------------------------------------------------------------
void UISystem::AddWidgetFromXML( XMLNode const & node )
{
	std::string childName = node.getName( );
	UIWidget * childWidget = UISystem::CreateWidgetFromName( childName, node );
	if( childWidget )
	{
		m_root->AddChild( childWidget );
	}
}