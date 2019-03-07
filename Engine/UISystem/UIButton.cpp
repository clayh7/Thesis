#include "Engine/UISystem/UIButton.hpp"

#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UIButton::s_UIButtonRegistration( "UIButton", &UIButton::CreateWidgetFromXML );
STATIC char const * UIButton::PROPERTY_BORDER_COLOR = "BorderColor";
STATIC char const * UIButton::PROPERTY_BORDER_SIZE = "BorderSize";
STATIC char const * UIButton::PROPERTY_BACKGROUND_COLOR = "BackgroundColor";
STATIC char const * UIButton::PROPERTY_TEXT_COLOR = "TextColor";
STATIC char const * UIButton::PROPERTY_TEXT = "Text";
STATIC char const * UIButton::PROPERTY_TEXT_SIZE = "TextSize";
STATIC char const * UIButton::PROPERTY_TEXT_ALIGNMENT = "TextAlignment";


//-------------------------------------------------------------------------------------------------
UIWidget * UIButton::CreateWidgetFromXML( XMLNode const & node )
{
	return new UIButton( node );
}


//-------------------------------------------------------------------------------------------------
UIButton::UIButton( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_borderMesh( nullptr )
	, m_border( nullptr )
	, m_background( nullptr )
	, m_text( nullptr )
{
	SetProperty( PROPERTY_BORDER_COLOR,		Color::BLACK );
	SetProperty( PROPERTY_BORDER_SIZE,		2.f );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::DARK_GREY,	eWidgetState_DISABLED );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::WHITE,		eWidgetState_ENABLED );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::LIGHT_GREEN,	eWidgetState_HIGHLIGHTED );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::DARK_GREEN,	eWidgetState_PRESSED );
	SetProperty( PROPERTY_TEXT_COLOR,		Color::BLACK );
	SetProperty( PROPERTY_TEXT,				"Button" );
	SetProperty( PROPERTY_TEXT_SIZE,		12.f );
	SetProperty( PROPERTY_TEXT_ALIGNMENT,	eAlignment_CENTER );

	SetProperty( PROPERTY_WIDTH, 150.f );
	SetProperty( PROPERTY_HEIGHT, 50.f );

	SetupRenderers( );
}


//-------------------------------------------------------------------------------------------------
UIButton::UIButton( XMLNode const & node )
	: UIButton( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UIButton::~UIButton( )
{
	delete m_borderMesh;
	m_borderMesh = nullptr;

	delete m_border;
	m_border = nullptr;

	delete m_background;
	m_background = nullptr;

	delete m_text;
	m_text = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIButton::Update( )
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
void UIButton::Render( ) const
{
	if( m_hidden )
	{
		return;
	}

	m_background->Render( );
	m_border->Render( );
	m_text->Render( );

	UIWidget::Render( );
}


//-------------------------------------------------------------------------------------------------
void UIButton::SetupRenderers( )
{
	//Dummy values to get them set up
	MeshBuilder borderBuilder;
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( -0.5f, -0.5f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ) );
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ), Vector3f( 0.5f, 0.5f, 0.f ) );

	m_borderMesh = new Mesh( &borderBuilder, eVertexType_PCU );
	m_border = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_background = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_text = new TextRenderer( "Default", Vector2f::ZERO );
}


//-------------------------------------------------------------------------------------------------
Vector2f UIButton::GetWorldPositionForAlignment( eAlignment const & textAlignment )
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
void UIButton::UpdateRenderers( )
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
void UIButton::CreateBorderMesh( Mesh * out_mesh )
{
	MeshBuilder borderBuilder;
	float borderSize = 0.f;
	GetProperty( PROPERTY_BORDER_SIZE, borderSize );

	Vector2f worldTL = GetWorldPosition( eAnchor_TOP_LEFT );
	Vector2f worldTR = GetWorldPosition( eAnchor_TOP_RIGHT );
	Vector2f worldBL = GetWorldPosition( eAnchor_BOTTOM_LEFT );
	Vector2f worldBR = GetWorldPosition( eAnchor_BOTTOM_RIGHT );

	Vector2f topLeft = worldTL + Vector2f( -borderSize, borderSize );
	Vector2f bottomRight = worldTR + Vector2f( borderSize, 0.f );
	borderBuilder.AddQuad( topLeft, bottomRight );

	topLeft = worldBL + Vector2f( -borderSize, 0.f );
	bottomRight = worldBR + Vector2f( borderSize, -borderSize );
	borderBuilder.AddQuad( topLeft, bottomRight );

	topLeft = worldTL + Vector2f( -borderSize, 0.f );
	bottomRight = worldBL + Vector2f( 0.f, 0.f );
	borderBuilder.AddQuad( topLeft, bottomRight );
	
	topLeft = worldTR + Vector2f( 0.f, 0.f );
	bottomRight = worldBR + Vector2f( borderSize, 0.f );
	borderBuilder.AddQuad( topLeft, bottomRight );

	out_mesh->Update( &borderBuilder );
}


//-------------------------------------------------------------------------------------------------
Transform UIButton::CalcBackgroundTransform( )
{
	Vector2f position = GetWorldPosition( eAnchor_CENTER );
	Vector2f scale = GetSize( );

	return Transform( position, Matrix4f( ), scale );
}


//-------------------------------------------------------------------------------------------------
void UIButton::PopulateFromXML( XMLNode const & node )
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