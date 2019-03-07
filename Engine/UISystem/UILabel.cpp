#include "Engine/UISystem/UILabel.hpp"

#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UILabel::s_UILabelRegistration( "UILabel", &UILabel::CreateWidgetFromXML );
STATIC char const * UILabel::PROPERTY_BACKGROUND_COLOR = "BackgroundColor";
STATIC char const * UILabel::PROPERTY_TEXT_COLOR = "TextColor";
STATIC char const * UILabel::PROPERTY_TEXT = "Text";
STATIC char const * UILabel::PROPERTY_TEXT_SIZE = "TextSize";
STATIC char const * UILabel::PROPERTY_TEXT_ALIGNMENT = "TextAlignment";


//-------------------------------------------------------------------------------------------------
UIWidget * UILabel::CreateWidgetFromXML( XMLNode const & node )
{
	return new UILabel( node );
}


//-------------------------------------------------------------------------------------------------
UILabel::UILabel( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_background( nullptr )
	, m_text( nullptr )
{
	//New Properties
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color( 0, 0, 0, 128 ),	eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_TEXT_COLOR,		Color::WHITE,			eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_TEXT,				"Label",				eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_TEXT_SIZE,		12.f,					eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_TEXT_ALIGNMENT,	eAlignment_CENTER,		eWidgetPropertySource_CODE );

	//Old Properties
	SetProperty( PROPERTY_WIDTH,	150.f,	eWidgetPropertySource_CODE );
	SetProperty( PROPERTY_HEIGHT,	50.f,	eWidgetPropertySource_CODE );

	SetupRenderers( );
}


//-------------------------------------------------------------------------------------------------
UILabel::UILabel( XMLNode const & node )
	: UILabel( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UILabel::~UILabel( )
{
	delete m_background;
	m_background = nullptr;

	delete m_text;
	m_text = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UILabel::Update( )
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
void UILabel::Render( ) const
{
	if( m_hidden )
	{
		return;
	}

	m_background->Render( );
	m_text->Render( );

	UIWidget::Render( );
}


//-------------------------------------------------------------------------------------------------
void UILabel::SetupRenderers( )
{
	m_background = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_text = new TextRenderer( "Default", Vector2f::ZERO );
}


//-------------------------------------------------------------------------------------------------
Vector2f UILabel::GetWorldPositionForAlignment( eAlignment const & textAlignment )
{
	switch( textAlignment )
	{
	case eAlignment_LEFT:
		return GetWorldPosition( eAnchor_LEFT );
	case eAlignment_CENTER:
		return GetWorldPosition( eAnchor_CENTER );
	case eAlignment_RIGHT:
		return GetWorldPosition( eAnchor_RIGHT );
	case eAlignment_NONE:
		return GetWorldPosition( eAnchor_CENTER );
	default:
		return GetWorldPosition( eAnchor_CENTER );
	}
}


//-------------------------------------------------------------------------------------------------
void UILabel::UpdateRenderers( )
{
	//Update Background
	Color backgroundColor;
	GetProperty( PROPERTY_BACKGROUND_COLOR, backgroundColor );

	Transform backgroundTransform = CalcBackgroundTransform( );

	m_background->SetUniform( "uColor", backgroundColor );
	m_background->SetTransform( backgroundTransform );
	m_background->Update( );

	//Update Text
	std::string text;
	GetProperty( PROPERTY_TEXT, text );

	Color textColor;
	GetProperty( PROPERTY_TEXT_COLOR, textColor );

	float textSize;
	GetProperty( PROPERTY_TEXT_SIZE, textSize );

	eAlignment textAlignment;
	GetProperty( PROPERTY_TEXT_ALIGNMENT, textAlignment );
	Vector2f textPosition = GetWorldPositionForAlignment( textAlignment );

	m_text->SetText( text );
	m_text->SetColor( textColor );
	m_text->SetSize( textSize );
	m_text->SetPosition( textPosition );
	m_text->SetAlignment( textAlignment );
	m_text->Update( );
}


//-------------------------------------------------------------------------------------------------
Transform UILabel::CalcBackgroundTransform( )
{
	Vector2f position = GetWorldPosition( eAnchor_CENTER );
	Vector2f scale = GetSize( );

	return Transform( position, Matrix4f( ), scale );
}


//-------------------------------------------------------------------------------------------------
void UILabel::PopulateFromXML( XMLNode const & node )
{
	//Set properties from XML
	for( int childPropertyIndex = 0; childPropertyIndex < node.nChildNode( ); ++childPropertyIndex )
	{
		XMLNode childProperty = node.getChildNode( childPropertyIndex );
		//Background
		if( strcmp( childProperty.getName( ), PROPERTY_BACKGROUND_COLOR ) == 0 )
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
		else if( strcmp( childProperty.getName( ), PROPERTY_TEXT ) == 0 )
		{
			std::string text = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_TEXT, text, ParseState( state ) );
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
	}
}