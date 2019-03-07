#include "Engine/UISystem/UITextField.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UITextField::s_UITextFieldRegistration( "UITextField", &UITextField::CreateWidgetFromXML );
STATIC char const * UITextField::PROPERTY_BORDER_COLOR = "BorderColor";
STATIC char const * UITextField::PROPERTY_BORDER_SELECTED_COLOR = "SelectedColor";
STATIC char const * UITextField::PROPERTY_BORDER_SIZE = "BorderSize";
STATIC char const * UITextField::PROPERTY_BACKGROUND_COLOR = "BackgroundColor";
STATIC char const * UITextField::PROPERTY_TEXT_COLOR = "TextColor";
STATIC char const * UITextField::PROPERTY_TEXT_SIZE = "TextSize";
STATIC char const * UITextField::PROPERTY_TEXT_ALIGNMENT = "TextAlignment";
STATIC char const * UITextField::PROPERTY_HINT_COLOR = "HintColor";
STATIC char const * UITextField::PROPERTY_HINT = "Hint";
STATIC char const * UITextField::PROPERTY_HINT_SIZE = "HintSize";
STATIC char const * UITextField::PROPERTY_HINT_ALIGNMENT = "HintAlignment";
STATIC char const * UITextField::PROPERTY_PASSWORD = "Password";
STATIC float const UITextField::BLINK_DELAY_SECONDS = 0.5f;


//-------------------------------------------------------------------------------------------------
UIWidget * UITextField::CreateWidgetFromXML( XMLNode const & node )
{
	return new UITextField( node );
}


//-------------------------------------------------------------------------------------------------
UITextField::UITextField( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_borderMesh( nullptr )
	, m_border( nullptr )
	, m_background( nullptr )
	, m_text( nullptr )
	, m_hint( nullptr )
	, m_content( "" )
	, m_blinkTimer( 0.f )
	, m_showBlinkingLine( false )
{
	SetProperty( PROPERTY_BORDER_COLOR, Color::BLACK );
	SetProperty( PROPERTY_BORDER_SIZE, 2.f );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::DARK_GREY, eWidgetState_DISABLED );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::WHITE, eWidgetState_ENABLED );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::LIGHT_GREEN, eWidgetState_HIGHLIGHTED );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::DARK_GREEN, eWidgetState_PRESSED );
	SetProperty( PROPERTY_TEXT_COLOR, Color::BLACK );
	SetProperty( PROPERTY_TEXT_SIZE, 12.f );
	SetProperty( PROPERTY_TEXT_ALIGNMENT, eAlignment_LEFT );
	SetProperty( PROPERTY_HINT_COLOR, Color::GREY );
	SetProperty( PROPERTY_HINT, "Text Field..." );
	SetProperty( PROPERTY_HINT_SIZE, 12.f );
	SetProperty( PROPERTY_HINT_ALIGNMENT, eAlignment_LEFT );
	SetProperty( PROPERTY_PASSWORD, false );

	SetProperty( PROPERTY_WIDTH, 200.f );
	SetProperty( PROPERTY_HEIGHT, 50.f );

	SetupRenderers( );

	EventSystem::RegisterEvent( Input::CHAR_TYPED_EVENT, this, &UITextField::OnAddChar );
	EventSystem::RegisterEvent( Input::BACKSPACE_EVENT, this, &UITextField::OnRemoveChar );
}


//-------------------------------------------------------------------------------------------------
UITextField::UITextField( XMLNode const & node )
	: UITextField( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UITextField::~UITextField( )
{
	EventSystem::Unregister( this );

	delete m_borderMesh;
	m_borderMesh = nullptr;

	delete m_border;
	m_border = nullptr;

	delete m_background;
	m_background = nullptr;

	delete m_text;
	m_text = nullptr;
	
	delete m_hint;
	m_hint = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UITextField::Update( )
{
	if( m_hidden )
	{
		return;
	}

	//Make blinking cursor at the end of the line
	UpdateBlinkingLine( );

	if( m_dirty )
	{
		UpdateRenderers( );
		m_dirty = false;
	}

	UIWidget::Update( );
}


//-------------------------------------------------------------------------------------------------
void UITextField::Render( ) const
{
	if( m_hidden )
	{
		return;
	}

	m_background->Render( );
	m_border->Render( );
	if( m_content == "" )
	{
		m_hint->Render( );
		m_text->Render( );
	}
	else
	{
		m_text->Render( );
	}

	UIWidget::Render( );
}


//-------------------------------------------------------------------------------------------------
std::string const & UITextField::GetText( ) const
{
	return m_content;
}


//-------------------------------------------------------------------------------------------------
void UITextField::SetText( std::string const & text )
{
	m_content = text;
}


//-------------------------------------------------------------------------------------------------
void UITextField::Select( ) const
{
	g_UISystem->Select( ( UIWidget* )this );
}


//-------------------------------------------------------------------------------------------------
void UITextField::UpdateBlinkingLine( )
{
	if( IsSelected( ) )
	{
		m_blinkTimer += Time::DELTA_SECONDS;
		if( m_blinkTimer > BLINK_DELAY_SECONDS )
		{
			while( m_blinkTimer > BLINK_DELAY_SECONDS )
			{
				m_blinkTimer -= BLINK_DELAY_SECONDS;
			}
			m_showBlinkingLine = !m_showBlinkingLine;
			Dirty( );
		}
	}
	else
	{
		m_showBlinkingLine = false;
	}
}


//-------------------------------------------------------------------------------------------------
void UITextField::SetupRenderers( )
{
	//Dummy values to get them set up
	MeshBuilder borderBuilder;
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( -0.5f, -0.5f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ) );
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ), Vector3f( 0.5f, 0.5f, 0.f ) );

	m_borderMesh = new Mesh( &borderBuilder, eVertexType_PCU );
	m_border = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_background = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_text = new TextRenderer( "", Vector2f::ZERO );
	m_hint = new TextRenderer( "Default", Vector2f::ZERO );
}


//-------------------------------------------------------------------------------------------------
void UITextField::UpdateRenderers( )
{
	//Update Border
	CreateBorderMesh( m_borderMesh );

	Color borderColor;
	if( IsSelected( ) )
	{
		GetProperty( PROPERTY_BORDER_SELECTED_COLOR, borderColor );
	}
	else
	{
		GetProperty( PROPERTY_BORDER_COLOR, borderColor );
	}

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

	//Update Text
	std::string text;
	Color textColor;
	float textSize;
	eAlignment textAlignment;
	Vector2f textPosition;

	GetProperty( PROPERTY_TEXT_COLOR, textColor );
	GetProperty( PROPERTY_TEXT_SIZE, textSize );
	GetProperty( PROPERTY_TEXT_ALIGNMENT, textAlignment );
	if( textAlignment == eAlignment_LEFT )
	{
		textPosition = GetWorldPosition( eAnchor_LEFT );
	}
	if( textAlignment == eAlignment_CENTER )
	{
		textPosition = GetWorldPosition( eAnchor_CENTER );
	}
	if( textAlignment == eAlignment_RIGHT )
	{
		textPosition = GetWorldPosition( eAnchor_RIGHT );
	}

	//Text content does not get updated with this Widget
	std::string textContent = GetDisplayContent( );
	m_text->SetText( textContent );
	m_text->SetColor( textColor );
	m_text->SetSize( textSize );
	m_text->SetAlignment( textAlignment );
	m_text->SetPosition( textPosition );
	m_text->Update( );

	//Update Hint
	std::string hint;
	Color hintColor;
	float hintSize;
	eAlignment hintAlignment;
	Vector2f hintPosition;

	GetProperty( PROPERTY_HINT, hint );
	GetProperty( PROPERTY_HINT_COLOR, hintColor );
	GetProperty( PROPERTY_HINT_SIZE, hintSize );
	GetProperty( PROPERTY_HINT_ALIGNMENT, hintAlignment );
	if( hintAlignment == eAlignment_LEFT )
	{
		hintPosition = GetWorldPosition( eAnchor_LEFT );
	}
	if( hintAlignment == eAlignment_CENTER )
	{
		hintPosition = GetWorldPosition( eAnchor_CENTER );
	}
	if( hintAlignment == eAlignment_RIGHT )
	{
		hintPosition = GetWorldPosition( eAnchor_RIGHT );
	}

	m_hint->SetText( hint );
	m_hint->SetColor( hintColor );
	m_hint->SetSize( hintSize );
	m_hint->SetAlignment( hintAlignment );
	m_hint->SetPosition( hintPosition );
	m_hint->Update( );
}


//-------------------------------------------------------------------------------------------------
void UITextField::CreateBorderMesh( Mesh * out_mesh )
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
Transform UITextField::CalcBackgroundTransform( )
{
	Vector2f position = GetWorldPosition( eAnchor_CENTER );
	Vector2f scale = GetSize( );

	return Transform( position, Matrix4f( ), scale );
}


//-------------------------------------------------------------------------------------------------
std::string UITextField::GetDisplayContent( ) const
{
	bool password = false;
	GetProperty( PROPERTY_PASSWORD, password );
	std::string display = m_content;
	std::string blinkingLine = ( ( m_showBlinkingLine ) ? "|" : "" );
	if( password )
	{
		display = "";
		for( size_t index = 0; index < m_content.size( ); ++index )
		{
			display += '*';
		}
	}
	return display + blinkingLine;
}


//-------------------------------------------------------------------------------------------------
void UITextField::PopulateFromXML( XMLNode const & node )
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
		else if( strcmp( childProperty.getName( ), PROPERTY_BORDER_SELECTED_COLOR ) == 0 )
		{
			Color selectedColor = ReadXMLAttribute( childProperty, STRING_VALUE, Color::WHITE );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_BORDER_SELECTED_COLOR, selectedColor, ParseState( state ) );
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
		//Text
		else if( strcmp( childProperty.getName( ), PROPERTY_TEXT_COLOR ) == 0 )
		{
			Color textColor = ReadXMLAttribute( childProperty, STRING_VALUE, Color::WHITE );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_TEXT_COLOR, textColor, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_TEXT_SIZE ) == 0 )
		{
			float textSize = ReadXMLAttribute( childProperty, STRING_VALUE, 12.f );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_TEXT_SIZE, textSize, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_TEXT_ALIGNMENT ) == 0 )
		{
			std::string alignment = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_TEXT_ALIGNMENT, ParseAlignment( alignment ), ParseState( state ) );
		}
		//Hint
		else if( strcmp( childProperty.getName( ), PROPERTY_HINT_COLOR ) == 0 )
		{
			Color hintColor = ReadXMLAttribute( childProperty, STRING_VALUE, Color::WHITE );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_HINT_COLOR, hintColor, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_HINT ) == 0 )
		{
			std::string hint = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_HINT, hint, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_HINT_SIZE ) == 0 )
		{
			float hintSize = ReadXMLAttribute( childProperty, STRING_VALUE, 12.f );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_HINT_SIZE, hintSize, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_HINT_ALIGNMENT ) == 0 )
		{
			std::string alignment = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_HINT_ALIGNMENT, ParseAlignment( alignment ), ParseState( state ) );
		}
		//Password
		else if( strcmp( childProperty.getName( ), PROPERTY_PASSWORD ) == 0 )
		{
			std::string password = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			bool parsePassword = !(strcmp( password.c_str( ), "false" ) == 0);
			SetProperty( PROPERTY_PASSWORD, parsePassword, ParseState( state ) );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UITextField::OnAddChar( NamedProperties & charTypedEvent )
{
	if( IsSelected( ) )
	{
		//Get character
		unsigned char asKey;
		charTypedEvent.Get( "asKey", asKey );

		//Add it to the end of the text
		m_content += asKey;
		//m_text->SetText( content );
		Dirty( );
	}
}


//-------------------------------------------------------------------------------------------------
void UITextField::OnRemoveChar( NamedProperties & )
{
	if( IsSelected( ) )
	{
		//Remove the last (far right) character from text
		if( m_content.size( ) > 0 )
		{
			m_content = m_content.substr( 0, m_content.size( ) - 1 );
			//m_text->SetText( content );
			Dirty( );
		}
	}
}