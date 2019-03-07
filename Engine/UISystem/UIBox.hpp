#pragma once

#include "Engine/UISystem/UIWidget.hpp"


//-------------------------------------------------------------------------------------------------
class Mesh;
class MeshRenderer;
class Transform;
class UIWidgetRegistration;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class UIBox : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static UIWidgetRegistration s_UIBoxRegistration;

public:
	static char const * PROPERTY_BORDER_COLOR;
	static char const * PROPERTY_BORDER_SIZE;
	static char const * PROPERTY_BACKGROUND_COLOR;

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

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UIBox( std::string const & name = "" );
	UIBox( XMLNode const & node );
	virtual ~UIBox( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;

protected:
	void SetupRenderers( );
	void UpdateRenderers( );
	void CreateBorderMesh( Mesh * out_mesh );
	Transform CalcBackgroundTransform( );
	void PopulateFromXML( XMLNode const & node );
};