#pragma once

#include "Engine/UISystem/UIWidget.hpp"


//-------------------------------------------------------------------------------------------------
class Mesh;
class MeshRenderer;
class Transform;
class UIItem;
class UIWidgetRegistration;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class UIContainer : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static UIWidgetRegistration s_UIContainerRegistration;

public:
	static char const * PROPERTY_BORDER_COLOR;
	static char const * PROPERTY_BORDER_SIZE;
	static char const * PROPERTY_BACKGROUND_COLOR;
	static char const * PROPERTY_GRID_SIZE;
	static char const * PROPERTY_ON_ADD_ITEM;

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
	UIContainer( std::string const & name = "" );
	UIContainer( XMLNode const & node );
	virtual ~UIContainer( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;
	Vector2f GetItemSize( ) const;
	Vector2f GetPositionForItem( UIItem const * childItem ) const;
	bool IsFull( ) const;
	bool IsEmpty( ) const;
	void AddItem( UIItem * item );

protected:
	void SetupRenderers( );
	void UpdateRenderers( );
	void CreateBorderMesh( Mesh * out_mesh );
	Transform CalcBackgroundTransform( );
	void PopulateFromXML( XMLNode const & node );
};