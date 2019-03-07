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
class UITextField : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static UIWidgetRegistration s_UITextFieldRegistration;
	static char const * PROPERTY_BORDER_COLOR;
	static char const * PROPERTY_BORDER_SELECTED_COLOR;
	static char const * PROPERTY_BORDER_SIZE;
	static char const * PROPERTY_BACKGROUND_COLOR;
	static char const * PROPERTY_TEXT_COLOR;
	static char const * PROPERTY_TEXT_SIZE;
	static char const * PROPERTY_TEXT_ALIGNMENT;
	static char const * PROPERTY_HINT_COLOR;
	static char const * PROPERTY_HINT;
	static char const * PROPERTY_HINT_SIZE;
	static char const * PROPERTY_HINT_ALIGNMENT;
	static char const * PROPERTY_PASSWORD;
	static float const BLINK_DELAY_SECONDS;

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
	TextRenderer * m_hint;

private:
	std::string m_content;
	float m_blinkTimer;
	bool m_showBlinkingLine;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UITextField( std::string const & name = "" );
	UITextField( XMLNode const & node );
	virtual ~UITextField( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;
	std::string const & GetText( ) const;
	void SetText( std::string const & text );
	void Select( ) const;

protected:
	void UpdateBlinkingLine( );
	void SetupRenderers( );
	void UpdateRenderers( );
	void CreateBorderMesh( Mesh * out_mesh );
	Transform CalcBackgroundTransform( );
	std::string GetDisplayContent( ) const;
	void PopulateFromXML( XMLNode const & node );
	
	void OnAddChar( NamedProperties & charTypedEvent );
	void OnRemoveChar( NamedProperties & );
};