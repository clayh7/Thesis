#include "Engine/UISystem/UIProgressBar.hpp"

#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UIProgressBar::s_UIProgressBarRegistration( "UIProgressBar", &UIProgressBar::CreateWidgetFromXML );
STATIC char const * UIProgressBar::PROPERTY_BORDER_COLOR = "BorderColor";
STATIC char const * UIProgressBar::PROPERTY_BORDER_SIZE = "BorderSize";
STATIC char const * UIProgressBar::PROPERTY_BAR_COLOR = "BarColor";
STATIC char const * UIProgressBar::PROPERTY_BAR_FILL_DIRECTION = "BarFillDirection";
STATIC char const * UIProgressBar::PROPERTY_BACKGROUND_COLOR = "BackgroundColor";
STATIC char const * UIProgressBar::PROPERTY_AMOUNT_BIND_EVENT = "AmountBindEvent";
STATIC char const * UIProgressBar::STRING_BIND_NAME = "name";


//-------------------------------------------------------------------------------------------------
UIWidget * UIProgressBar::CreateWidgetFromXML( XMLNode const & node )
{
	return new UIProgressBar( node );
}


//-------------------------------------------------------------------------------------------------
UIProgressBar::UIProgressBar( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_borderMesh( nullptr )
	, m_border( nullptr )
	, m_bar( nullptr )
	, m_background( nullptr )
	, m_text( nullptr )
	, m_amount( 1.f )
{
	SetProperty( PROPERTY_BORDER_COLOR,			Color::BLACK );
	SetProperty( PROPERTY_BORDER_SIZE,			-2.f );
	SetProperty( PROPERTY_BAR_COLOR,			Color::GREEN );
	SetProperty( PROPERTY_BAR_FILL_DIRECTION,	eFillDirection_LEFT_TO_RIGHT );
	SetProperty( PROPERTY_AMOUNT_BIND_EVENT,	"UIProgressBarAmountBindEvent" );
	SetProperty( PROPERTY_BACKGROUND_COLOR,		Color::BLACK );

	SetProperty( PROPERTY_WIDTH, 200.f );
	SetProperty( PROPERTY_HEIGHT, 50.f );

	SetupRenderers( );
}


//-------------------------------------------------------------------------------------------------
UIProgressBar::UIProgressBar( XMLNode const & node )
	: UIProgressBar( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UIProgressBar::~UIProgressBar( )
{
	delete m_borderMesh;
	m_borderMesh = nullptr;

	delete m_border;
	m_border = nullptr;

	delete m_bar;
	m_bar = nullptr;

	delete m_background;
	m_background = nullptr;

	delete m_text;
	m_text = nullptr;

	EventSystem::Unregister( this );
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::Update( )
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
void UIProgressBar::Render( ) const
{
	if( m_hidden )
	{
		return;
	}

	m_background->Render( );
	m_bar->Render( );
	m_border->Render( );
	m_text->Render( );

	UIWidget::Render( );
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::SetAmount( float amount )
{
	if( m_amount != amount )
	{
		m_amount = Clamp( amount, 0.f, 1.f );
		Dirty( );
	}
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::SetAmount( float amount, float max )
{
	if( max == 0.f )
	{
		SetAmount( 0.f );
	}
	else
	{
		SetAmount( amount / max );
	}
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::SetAmount( int amount, int max )
{
	SetAmount( (float) amount, (float) max );
}


//-------------------------------------------------------------------------------------------------
eFillDirection UIProgressBar::GetFillDirection( ) const
{
	eFillDirection fillDirection = eFillDirection_LEFT_TO_RIGHT;
	GetProperty( PROPERTY_BAR_FILL_DIRECTION, fillDirection );
	return fillDirection;
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::OnSetAmount( NamedProperties & properties )
{
	float amount = 0.f;
	properties.Get( m_amountBindName, amount );
	SetAmount( amount );
	Dirty( );
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::SetupRenderers( )
{
	//Dummy values to get them set up
	MeshBuilder borderBuilder;
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( -0.5f, -0.5f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ) );
	borderBuilder.AddTriangle( Vector3f( -0.5f, 5.f, 0.f ), Vector3f( 0.5f, -0.5f, 0.f ), Vector3f( 0.5f, 0.5f, 0.f ) );

	m_borderMesh = new Mesh( &borderBuilder, eVertexType_PCU );
	m_border = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_bar = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_background = new MeshRenderer( eMeshShape_QUAD, Transform( ), RenderState::BASIC_2D );
	m_text = new TextRenderer( "", Vector2f::ZERO );
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::UpdateRenderers( )
{
	//Update Border
	CreateBorderMesh( m_borderMesh );

	Color borderColor;
	GetProperty( PROPERTY_BORDER_COLOR, borderColor );

	m_border->SetMesh( m_borderMesh );
	m_border->SetUniform( "uColor", borderColor );
	m_border->Update( );

	//Update Bar
	Color barColor;
	GetProperty( PROPERTY_BAR_COLOR, barColor );

	Transform barTransform = CalcBarTransform( );

	m_bar->SetUniform( "uColor", barColor );
	m_bar->SetTransform( barTransform );
	m_bar->Update( );

	//Update Background
	Color backgroundColor;
	GetProperty( PROPERTY_BACKGROUND_COLOR, backgroundColor );

	Transform backgroundTransform = CalcBackgroundTransform( );

	m_background->SetUniform( "uColor", backgroundColor );
	m_background->SetTransform( backgroundTransform );
	m_background->Update( );

	m_text->SetAlignment( eAlignment_CENTER );
	m_text->SetPosition( GetWorldPosition( eAnchor_CENTER ) );
	m_text->Update( );
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::CreateBorderMesh( Mesh * out_mesh )
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
Transform UIProgressBar::CalcBarTransform( )
{
	//Figure out size
	eFillDirection fillDirection = GetFillDirection( );
	Vector2f scale = GetSize( );
	if( fillDirection == eFillDirection_LEFT_TO_RIGHT ||
		fillDirection == eFillDirection_RIGHT_TO_LEFT )
	{
		scale.x *= m_amount;
	}
	else
	{
		scale.y *= m_amount;
	}

	//Figure out location
	Vector2f position = Vector2f::ZERO;
	if( fillDirection == eFillDirection_LEFT_TO_RIGHT )
	{
		position = GetWorldPosition( eAnchor_LEFT );
		position.x += scale.x / 2.f;
	}
	else if( fillDirection == eFillDirection_RIGHT_TO_LEFT )
	{
		position = GetWorldPosition( eAnchor_RIGHT );
		position.x -= scale.x / 2.f;
	}
	else if( fillDirection == eFillDirection_BOTTOM_TO_TOP )
	{
		position = GetWorldPosition( eAnchor_BOTTOM );
		position.y += scale.y / 2.f;
	}
	else if( fillDirection == eFillDirection_TOP_TO_BOTTOM )
	{
		position = GetWorldPosition( eAnchor_TOP );
		position.y -= scale.y / 2.f;
	}

	//Convert to local space
	return Transform( position, Matrix4f( ), scale );
}


//-------------------------------------------------------------------------------------------------
Transform UIProgressBar::CalcBackgroundTransform( )
{
	Vector2f position = GetWorldPosition( eAnchor_CENTER );
	Vector2f scale = GetSize( );

	return Transform( position, Matrix4f( ), scale );
}


//-------------------------------------------------------------------------------------------------
void UIProgressBar::PopulateFromXML( XMLNode const & node )
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
		//Bar
		else if( strcmp( childProperty.getName( ), PROPERTY_BAR_COLOR ) == 0 )
		{
			Color barColor = ReadXMLAttribute( childProperty, STRING_VALUE, Color::BLACK );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_BAR_COLOR, barColor, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_BAR_FILL_DIRECTION ) == 0 )
		{
			std::string barFillDirection = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_BAR_FILL_DIRECTION, ParseFillDirection( barFillDirection ), ParseState( state ) );
		}
		//Background
		else if( strcmp( childProperty.getName( ), PROPERTY_BACKGROUND_COLOR ) == 0 )
		{
			Color backgroundColor = ReadXMLAttribute( childProperty, STRING_VALUE, Color::WHITE );
			std::string state = ReadXMLAttribute( childProperty, STRING_STATE, "" );
			SetProperty( PROPERTY_BACKGROUND_COLOR, backgroundColor, ParseState( state ) );
		}
		else if( strcmp( childProperty.getName( ), PROPERTY_AMOUNT_BIND_EVENT ) == 0 )
		{
			std::string amountBindEvent = ReadXMLAttribute( childProperty, STRING_VALUE, "" );
			EventSystem::RegisterEvent( amountBindEvent, this, &UIProgressBar::OnSetAmount );
			m_amountBindName = ReadXMLAttribute( childProperty, STRING_BIND_NAME, "amount" );
		}
	}
}