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
class UIProgressBar : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static UIWidgetRegistration s_UIProgressBarRegistration;

public:
	static char const * PROPERTY_BORDER_COLOR;
	static char const * PROPERTY_BORDER_SIZE;
	static char const * PROPERTY_BAR_COLOR;
	static char const * PROPERTY_BAR_FILL_DIRECTION;
	static char const * PROPERTY_BACKGROUND_COLOR;
	static char const * PROPERTY_AMOUNT;
	static char const * PROPERTY_AMOUNT_BIND_EVENT;
	static char const * STRING_BIND_NAME;

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
	MeshRenderer * m_bar;
	MeshRenderer * m_background;
	TextRenderer * m_text;
	float m_amount;
	std::string m_amountBindName;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UIProgressBar( std::string const & name = "" );
	UIProgressBar( XMLNode const & node );
	virtual ~UIProgressBar( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;
	void SetAmount( float amount );
	void SetAmount( float amount, float max );
	void SetAmount( int amount, int max );
	eFillDirection GetFillDirection( ) const;
	void OnSetAmount( NamedProperties & properties );

protected:
	void SetupRenderers( );
	void UpdateRenderers( );
	void CreateBorderMesh( Mesh * out_mesh );
	Transform CalcBarTransform( );
	Transform CalcBackgroundTransform( );
	void PopulateFromXML( XMLNode const & node );
};