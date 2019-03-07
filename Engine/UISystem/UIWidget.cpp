#include "Engine/UISystem/UIWidget.hpp"

#include "Engine/AudioSystem/Audio.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UIWidget::s_UIWidgetRegistration( "UIWidget", &UIWidget::CreateWidgetFromXML );
STATIC size_t const UIWidget::INVALID_UID = 0;
STATIC char const * UIWidget::PROPERTY_HIDDEN = "Hidden";
STATIC char const * UIWidget::PROPERTY_ANCHOR = "Anchor";
STATIC char const * UIWidget::PROPERTY_DOCK = "Dock";
STATIC char const * UIWidget::PROPERTY_OFFSET = "Offset";
STATIC char const * UIWidget::PROPERTY_WIDTH = "Width";
STATIC char const * UIWidget::PROPERTY_HEIGHT = "Height";
STATIC char const * UIWidget::PROPERTY_SOUND = "Sound";
STATIC char const * UIWidget::PROPERTY_ON_HIGHLIGHT = "OnHighlight";
STATIC char const * UIWidget::PROPERTY_ON_PRESSED = "OnPressed";
STATIC char const * UIWidget::PROPERTY_ON_RELEASED = "OnReleased";
STATIC char const * UIWidget::STRING_VALUE = "value";
STATIC char const * UIWidget::STRING_STATE = "state";
STATIC size_t UIWidget::s_currentUID = 1;


//-------------------------------------------------------------------------------------------------
UIWidget * UIWidget::CreateWidgetFromXML( XMLNode const & node )
{
	return new UIWidget( node );
}


//-------------------------------------------------------------------------------------------------
UIWidget::UIWidget( std::string const & name /*= ""*/ )
	: m_parent( nullptr )
	, m_name( name )
	, m_state( eWidgetState_ENABLED )
	, m_lastSize( Vector2f::ZERO )
	, m_hidden( false )
	, m_dirty( true )
	, m_uid( s_currentUID++ )
	, m_recalculateSize( true )
{
	SetProperty( PROPERTY_ANCHOR,	eAnchor_CENTER,	eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_DOCK,		eDock_NONE,		eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_OFFSET,	Vector2f::ZERO,	eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_WIDTH,	100.f,			eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_HEIGHT,	100.f,			eWidgetPropertySource_CODE );
}


//-------------------------------------------------------------------------------------------------
UIWidget::UIWidget( XMLNode const & node )
	: UIWidget( )
{
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UIWidget::~UIWidget( )
{
	for( UIWidget * child : m_children )
	{
		delete child;
		child = nullptr;
	}
	m_children.clear( );
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Update( )
{
	if( m_hidden )
	{
		return;
	}

	for( UIWidget * child : m_children )
	{
		child->Update( );
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Render( ) const
{
	if( m_hidden )
	{
		return;
	}

	for( UIWidget const * child : m_children )
	{
		child->Render( );
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::AddChild( UIWidget * widget )
{
	if( widget->m_parent )
	{
		widget->m_parent->RemoveChild( widget );
	}
	m_children.push_back( widget );
	widget->m_parent = this;
	Dirty( );
}


//-------------------------------------------------------------------------------------------------
void UIWidget::RemoveChild( UIWidget * widget )
{
	for( auto childIter = m_children.begin( ); childIter != m_children.end( ); ++childIter )
	{
		UIWidget * child = *childIter;
		if( child == widget )
		{
			widget->m_parent = nullptr;
			m_children.erase( childIter );
			Dirty( );
			return;
		}
	}
}


//-------------------------------------------------------------------------------------------------
UIWidget * UIWidget::GetChild( size_t childIndex )
{
	return m_children[childIndex];
}


//-------------------------------------------------------------------------------------------------
void UIWidget::ChangeState( eWidgetState const & state )
{
	ASSERT_RECOVERABLE( m_state != state, "Do not change to the same state" );
	ExitState( m_state );
	m_state = state;
	EnterState( m_state );
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Highlight( )
{
	if( m_state == eWidgetState_ENABLED )
	{
		std::string highlightEventName = "";
		GetProperty( PROPERTY_ON_HIGHLIGHT, highlightEventName );
		EventSystem::TriggerEvent( highlightEventName );

		ChangeState( eWidgetState_HIGHLIGHTED );
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Unhighlight( )
{
	if( m_state == eWidgetState_HIGHLIGHTED || m_state == eWidgetState_PRESSED )
	{
		ChangeState( eWidgetState_ENABLED );
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Press( )
{
	if( m_state == eWidgetState_HIGHLIGHTED )
	{
		std::string pressedEventName = "";
		GetProperty( PROPERTY_ON_PRESSED, pressedEventName );
		EventSystem::TriggerEvent( pressedEventName );

		ChangeState( eWidgetState_PRESSED );
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Release( )
{
	if( m_state == eWidgetState_PRESSED )
	{
		std::string releasedEventName = "";
		GetProperty( PROPERTY_ON_RELEASED, releasedEventName );
		EventSystem::TriggerEvent( releasedEventName );

		ChangeState( eWidgetState_HIGHLIGHTED );
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Disable( )
{
	if( m_state != eWidgetState_DISABLED )
	{
		ChangeState( eWidgetState_DISABLED );
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::Enable( )
{
	if( m_state == eWidgetState_DISABLED )
	{
		ChangeState( eWidgetState_ENABLED );
	}
}


//-------------------------------------------------------------------------------------------------
//Being dirty means I need to go and update my visuals
void UIWidget::Dirty( )
{
	m_dirty = true;
	m_recalculateSize = true;
	for( size_t childIndex = 0; childIndex < m_children.size(); ++childIndex )
	{
		UIWidget * child = m_children[childIndex];
		if( child )
		{
			child->Dirty( );
		}
	}
}


//-------------------------------------------------------------------------------------------------
UIWidget * UIWidget::FindWidgetUnderPosition( Vector2f const & position )
{
	if( m_hidden )
	{
		return nullptr;
	}

	//Go through children in reverse order, find the first one that contains the cursor
	for( int childIndex = (int) m_children.size( ) - 1; childIndex >= 0; --childIndex )
	{
		UIWidget * child = m_children[childIndex];
		UIWidget * foundWidget = child->FindWidgetUnderPosition( position );
		if( foundWidget )
		{
			return foundWidget;
		}
	}

	//If it's not in my children, check myself
	if( IsPointInside( position ) )
	{
		return this;
	}
	else
	{
		return nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
UIWidget * UIWidget::GetParent( ) const
{
	return m_parent;
}


//-------------------------------------------------------------------------------------------------
Vector2f UIWidget::GetSize( ) const
{
	if( !m_recalculateSize )
	{
		return m_lastSize;
	}

	Vector2f currentSize = Vector2f::ZERO;
	GetProperty( PROPERTY_WIDTH, currentSize.x );
	GetProperty( PROPERTY_HEIGHT, currentSize.y );

	eDock currentDock = GetDock( );

	//Match parent for docking
	if( m_parent )
	{
		if( currentDock == eDock_BOTTOM )
		{
			Vector2f parentSize = m_parent->GetSizeForChild( this );
			currentSize.x = parentSize.x;
		}
		else if( currentDock == eDock_TOP )
		{
			Vector2f parentSize = m_parent->GetSizeForChild( this );
			currentSize.x = parentSize.x;
		}
		else if( currentDock == eDock_LEFT )
		{
			Vector2f parentSize = m_parent->GetSizeForChild( this );
			currentSize.y = parentSize.y;
		}
		else if( currentDock == eDock_RIGHT )
		{
			Vector2f parentSize = m_parent->GetSizeForChild( this );
			currentSize.y = parentSize.y;
		}
		else if( currentDock == eDock_FILL )
		{
			Vector2f parentSize = m_parent->GetSizeForChild( this );
			currentSize = parentSize;
		}
	}

	if( m_recalculateSize )
	{
		m_recalculateSize = false;
		m_lastSize = currentSize;
	}

	return currentSize;
}


//-------------------------------------------------------------------------------------------------
eWidgetState UIWidget::GetState( ) const
{
	return m_state;
}


//-------------------------------------------------------------------------------------------------
size_t UIWidget::GetUID( ) const
{
	return m_uid;
}


//-------------------------------------------------------------------------------------------------
void UIWidget::SetParent( UIWidget * widget )
{
	widget->AddChild( this );
}


//-------------------------------------------------------------------------------------------------
void UIWidget::SetHidden( bool isHidden )
{
	m_hidden = isHidden;
}


//-------------------------------------------------------------------------------------------------
void UIWidget::EnterState( eWidgetState const & state )
{
	//It feels wrong to dirty everything any time it enters a new state
	Dirty( );
	PlayUISound( );

	switch( state )
	{
	case eWidgetState_HIGHLIGHTED:
	{
		break;
	}
	case eWidgetState_PRESSED:
	{
		break;
	}
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::ExitState( eWidgetState const & state )
{
	switch( state )
	{
	case eWidgetState_PRESSED:
		break;
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::UpdateState( eWidgetState const & )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void UIWidget::PopulateFromXML( XMLNode const & node )
{
	//Set Name
	m_name = ReadXMLAttribute( node, "name", UISystem::DEFAULT_NAME );

	//Recurse and create children
	for( int childIndex = 0; childIndex < node.nChildNode( ); ++childIndex )
	{
		XMLNode const & childData = node.getChildNode( childIndex );
		std::string childName = childData.getName( );
		UIWidget * childWidget = UISystem::CreateWidgetFromName( childName, childData );
		if( childWidget )
		{
			AddChild( childWidget );
		}
	}

	//Set properties from XML
	for( int childPropertyIndex = 0; childPropertyIndex < node.nChildNode( ); ++childPropertyIndex )
	{
		XMLNode childProperty = node.getChildNode( childPropertyIndex );
		//Set hidden to be true or false immediately if property is present
		if( strcmp( childProperty.getName( ), PROPERTY_HIDDEN ) == 0 )
		{
			std::string hidden = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			m_hidden = strcmp( hidden.c_str( ), "false" ) != 0;
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_ANCHOR ) == 0 )
		{
			std::string anchor = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_ANCHOR, ParseAnchor( anchor ), ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_DOCK ) == 0 )
		{
			std::string dock = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_DOCK, ParseDock( dock ), ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_OFFSET ) == 0 )
		{
			Vector2f offset = ReadXMLAttribute( childProperty, STRING_VALUE, Vector2f::ZERO );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_OFFSET, offset, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_WIDTH ) == 0 )
		{
			float width = ReadXMLAttribute( childProperty, STRING_VALUE, 100.f );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_WIDTH, width, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_HEIGHT ) == 0 )
		{
			float height = ReadXMLAttribute( childProperty, STRING_VALUE, 100.f );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_HEIGHT, height, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_SOUND ) == 0 )
		{
			std::string sound = ReadXMLAttribute( childProperty, STRING_VALUE, "error" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_SOUND, sound, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_ON_HIGHLIGHT ) == 0 )
		{
			std::string onHighlight = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_ON_HIGHLIGHT, onHighlight, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_ON_PRESSED ) == 0 )
		{
			std::string onPressed = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_ON_PRESSED, onPressed, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_ON_RELEASED ) == 0 )
		{
			std::string onReleased = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_ON_RELEASED, onReleased, ParseState( state ) );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UIWidget::PlayUISound( )
{
	std::string soundFile;
	GetProperty( PROPERTY_SOUND, soundFile );
	SoundID sound = g_AudioSystem->CreateOrGetSound( Stringf( "Data/Audio/%s", soundFile.c_str( ) ) );
	if( sound != BAudio::MISSING_SOUND_ID )
	{
		g_AudioSystem->PlayAudio( sound );
	}
}


//-------------------------------------------------------------------------------------------------
bool UIWidget::IsPointInside( Vector2f const & position ) const
{
	//Get bounds
	Vector2f bottomLeft = GetWorldPosition( eAnchor_BOTTOM_LEFT );
	Vector2f topRight = GetWorldPosition( eAnchor_TOP_RIGHT );

	//Is within bounds
	if( position.x > bottomLeft.x && position.y > bottomLeft.y )
	{
		if( position.x < topRight.x && position.y < topRight.y )
		{
			return true;
		}
	}

	//Not within bounds
	return false;
}


//-------------------------------------------------------------------------------------------------
Vector2f UIWidget::GetSizeForChild( UIWidget const * forChild /*= nullptr */ ) const
{
	Vector2f currentSize = GetSize( );

	//Shrink space for docking
	if( forChild != nullptr )
	{
		for( UIWidget * child : m_children )
		{
			if( child == forChild )
			{
				return currentSize;
			}

			Vector2f childSize = child->GetSize( );
			eDock childDock = child->GetDock( );
			if( childDock == eDock_BOTTOM )
			{
				currentSize.y -= childSize.y;
			}
			else if( childDock == eDock_TOP )
			{
				currentSize.y -= childSize.y;
			}
			else if( childDock == eDock_LEFT )
			{
				currentSize.x -= childSize.x;
			}
			else if( childDock == eDock_RIGHT )
			{
				currentSize.x -= childSize.x;
			}
			else if( childDock == eDock_FILL )
			{
				currentSize -= childSize;
			}
		}

		ASSERT_RECOVERABLE( true, "GetSize (forChild) is only intended to be used with parents and children" );
		return Vector2f::ZERO;
	}

	return currentSize;
}


//-------------------------------------------------------------------------------------------------
Vector2f UIWidget::GetLocalPosition( eAnchor const & anchor ) const
{
	Vector2f currentSize = GetSize( );

	Vector2f result;
	switch( anchor )
	{
	case eAnchor_TOP_LEFT:
		result = Vector2f( 0.f, currentSize.y );
		break;

	case eAnchor_TOP:
		result = Vector2f( currentSize.x / 2.f, currentSize.y );
		break;

	case eAnchor_TOP_RIGHT:
		result = Vector2f( currentSize.x, currentSize.y );
		break;

	case eAnchor_LEFT:
		result = Vector2f( 0.f, currentSize.y / 2.f );
		break;

	case eAnchor_CENTER:
		result = Vector2f( currentSize.x / 2.f, currentSize.y / 2.f );
		break;

	case eAnchor_RIGHT:
		result = Vector2f( currentSize.x, currentSize.y / 2.f );
		break;

	case eAnchor_BOTTOM_LEFT:
		result = Vector2f( 0.f, 0.f );
		break;

	case eAnchor_BOTTOM:
		result = Vector2f( currentSize.x / 2.f, 0.f );
		break;

	case eAnchor_BOTTOM_RIGHT:
		result = Vector2f( currentSize.x, 0.f );
		break;

	default:
		result = Vector2f::ZERO;
		break;
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector2f UIWidget::GetWorldPosition( eAnchor const & anchor, UIWidget const * forChild /*= nullptr*/ ) const
{
	eAnchor currentAnchor = GetAnchor( );
	Vector2f currentOffset = GetOffset( );

	Vector2f currentWorldPosition = GetLocalPosition( anchor ) + currentOffset;

	//(basically) If we're not the root node
	if( m_parent )
	{
		currentWorldPosition += m_parent->GetWorldPosition( currentAnchor, this ) - GetLocalPosition( currentAnchor );
	}

	//Shift position for docking children
	if( forChild != nullptr )
	{
		for( UIWidget * child : m_children )
		{
			if( child == forChild )
			{
				return currentWorldPosition;
			}
			
			Vector2f childSize = child->GetSize( );
			eDock childDock = child->GetDock( );
			eDock checkDock = forChild->GetDock( ); //The docking of the child you are trying to find
			if( childDock == eDock_BOTTOM )
			{
				if( checkDock == eDock_BOTTOM )
				{
					currentWorldPosition.y += childSize.y;
				}
				else if( checkDock == eDock_LEFT
					|| checkDock == eDock_RIGHT
					|| checkDock == eDock_FILL )
				{
					currentWorldPosition.y += childSize.y / 2.f;
				}
			}
			else if( childDock == eDock_TOP )
			{
				if( checkDock == eDock_TOP )
				{
					currentWorldPosition.y -= childSize.y;
				}
				else if( checkDock == eDock_LEFT
					|| checkDock == eDock_RIGHT
					|| checkDock == eDock_FILL )
				{
					currentWorldPosition.y -= childSize.y / 2.f;
				}
			}
			else if( childDock == eDock_LEFT )
			{
				if( checkDock == eDock_LEFT )
				{
					currentWorldPosition.x += childSize.x;
				}
				else if( checkDock == eDock_TOP
					|| checkDock == eDock_BOTTOM
					|| checkDock == eDock_FILL )
				{
					currentWorldPosition.x += childSize.x / 2.f;
				}
			}
			else if( childDock == eDock_RIGHT )
			{
				if( checkDock == eDock_RIGHT )
				{
					currentWorldPosition.x -= childSize.x;
				}
				else if( checkDock == eDock_TOP
					|| checkDock == eDock_BOTTOM
					|| checkDock == eDock_FILL )
				{
					currentWorldPosition.x -= childSize.x / 2.f;
				}
			}
		}
	
		ASSERT_RECOVERABLE( true, "GetWorldPosition (forChild) is only intended to be used with parents and children" );
		return Vector2f::ZERO;
	}

	return currentWorldPosition;
}


//-------------------------------------------------------------------------------------------------
eAnchor UIWidget::GetAnchor( ) const
{
	eAnchor currentAnchor = eAnchor_CENTER;
	GetProperty( PROPERTY_ANCHOR, currentAnchor );

	eDock currentDock = GetDock( );

	//Adjust for specific docking
	if( currentDock == eDock_BOTTOM )
	{
		currentAnchor = eAnchor_BOTTOM;
	}
	else if( currentDock == eDock_TOP )
	{
		currentAnchor = eAnchor_TOP;
	}
	else if( currentDock == eDock_LEFT )
	{
		currentAnchor = eAnchor_LEFT;
	}
	else if( currentDock == eDock_RIGHT )
	{
		currentAnchor = eAnchor_RIGHT;
	}
	else if( currentDock == eDock_FILL )
	{
		currentAnchor = eAnchor_CENTER;
	}

	return currentAnchor;
}


//-------------------------------------------------------------------------------------------------
Vector2f UIWidget::GetOffset( ) const
{
	Vector2f currentOffset = Vector2f::ZERO;
	GetProperty( PROPERTY_OFFSET, currentOffset );

	eDock currentDock = GetDock( );

	//No offset if you are docked
	if( currentDock != eDock_NONE )
	{
		currentOffset = Vector2f::ZERO;
	}

	return currentOffset;
}


//-------------------------------------------------------------------------------------------------
eDock UIWidget::GetDock( ) const
{
	eDock currentDock = eDock_NONE;
	GetProperty( PROPERTY_DOCK, currentDock );
	
	return currentDock;
}


//-------------------------------------------------------------------------------------------------
bool UIWidget::IsSelected( ) const
{
	return g_UISystem->GetSelectedWidget( ) == this;
}