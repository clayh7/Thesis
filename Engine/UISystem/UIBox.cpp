#include "Engine/UISystem/UIBox.hpp"

#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/UISystem/UISystem.hpp"
#include "Engine/UISystem/UICommon.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC UIWidgetRegistration UIBox::s_UIBoxRegistration( "UIBox", &UIBox::CreateWidgetFromXML );
STATIC char const * UIBox::PROPERTY_BORDER_COLOR = "BorderColor";
STATIC char const * UIBox::PROPERTY_BORDER_SIZE = "BorderSize";
STATIC char const * UIBox::PROPERTY_BACKGROUND_COLOR = "BackgroundColor";


//-------------------------------------------------------------------------------------------------
UIWidget * UIBox::CreateWidgetFromXML( XMLNode const & node )
{
	return new UIBox( node );
}


//-------------------------------------------------------------------------------------------------
UIBox::UIBox( std::string const & name /*= ""*/ )
	: UIWidget( name )
	, m_borderMesh( nullptr )
	, m_border( nullptr )
	, m_background( nullptr )
{
	SetProperty( PROPERTY_BORDER_COLOR, Color::BLACK );
	SetProperty( PROPERTY_BORDER_SIZE, 0.f );
	SetProperty( PROPERTY_BACKGROUND_COLOR, Color::WHITE );

	SetupRenderers( );
}


//-------------------------------------------------------------------------------------------------
UIBox::UIBox( XMLNode const & node )
	: UIBox( )
{
	UIWidget::PopulateFromXML( node );
	PopulateFromXML( node );
}


//-------------------------------------------------------------------------------------------------
UIBox::~UIBox( )
{
	delete m_borderMesh;
	m_borderMesh = nullptr;

	delete m_border;
	m_border = nullptr;

	delete m_background;
	m_background = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UIBox::Update( )
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
void UIBox::Render( ) const
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
void UIBox::SetupRenderers( )
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
void UIBox::UpdateRenderers( )
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
void UIBox::CreateBorderMesh( Mesh * out_mesh )
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
Transform UIBox::CalcBackgroundTransform( )
{
	Vector2f position = GetWorldPosition( eAnchor_CENTER );
	Vector2f scale = GetSize( );

	return Transform( position, Matrix4f( ), scale );
}


//-------------------------------------------------------------------------------------------------
void UIBox::PopulateFromXML( XMLNode const & node )
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
	}
}