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
class UILabel : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static UIWidgetRegistration s_UILabelRegistration;
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
	MeshRenderer * m_background;
	TextRenderer * m_text;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UILabel( std::string const & name = "" );
	UILabel( XMLNode const & node );
	virtual ~UILabel( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;

protected:
	void SetupRenderers( );
	Vector2f GetWorldPositionForAlignment( eAlignment const & textAlignment );
	void UpdateRenderers( );
	Transform CalcBackgroundTransform( );
	void PopulateFromXML( XMLNode const & node );
};