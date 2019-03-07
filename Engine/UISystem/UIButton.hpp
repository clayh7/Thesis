#pragma once

#include "Engine/UISystem/UIWidget.hpp"


//-------------------------------------------------------------------------------------------------
class Mesh;
class MeshRenderer;
class TextRenderer;
class Transform;
class UIWidgetRegistration;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class UIButton : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static UIWidgetRegistration s_UIButtonRegistration;
	static char const * PROPERTY_BORDER_COLOR;
	static char const * PROPERTY_BORDER_SIZE;
	static char const * PROPERTY_BACKGROUND_COLOR;
	static char const * PROPERTY_TEXT_COLOR;
	static char const * PROPERTY_TEXT;
	static char const * PROPERTY_TEXT_SIZE;
	static char const * PROPERTY_TEXT_ALIGNMENT;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static UIWidget * CreateWidgetFromXML( XMLNode const & node );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Mesh * m_borderMesh;
	MeshRenderer * m_border;
	MeshRenderer * m_background;
	TextRenderer * m_text;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UIButton( std::string const & name = "" );
	UIButton( XMLNode const & node );
	virtual ~UIButton( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;

protected:
	void SetupRenderers( );
	Vector2f GetWorldPositionForAlignment( eAlignment const & textAlignment );
	void UpdateRenderers( );
	void CreateBorderMesh( Mesh * out_mesh );
	Transform CalcBackgroundTransform( );
	void PopulateFromXML( XMLNode const & node );
};