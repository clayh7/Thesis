#pragma once

#include "Engine/UISystem/UIWidget.hpp"


//-------------------------------------------------------------------------------------------------
class Mesh;
class Material;
class MeshRenderer;
class NamedProperties;
class Transform;
class SpriteResource;
class UIWidgetRegistration;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class UIItem : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static UIWidgetRegistration s_UIItemRegistration;

public:
	static char const * DEFAULT_SPRIE_ID;
	static char const * EVENT_PICK_UP_ITEM;
	static char const * EVENT_DROP_ITEM;
	static char const * PROPERTY_SPRITE_ID;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static UIWidget * CreateWidgetFromXML( XMLNode const & node );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	SpriteResource const * m_spriteData;
	Mesh * m_quadMesh;
	MeshRenderer * m_quad;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	UIItem( std::string const & name = "" );
	UIItem( XMLNode const & node );
	virtual ~UIItem( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;
	virtual Vector2f GetSize( ) const override;

protected:
	void SetupRenderers( );
	void UpdateRenderers( );
	void CreateSpriteMesh( Mesh * out_mesh );
	Transform CalcItemTransform( );
	void PopulateFromXML( XMLNode const & node );
	virtual Vector2f GetWorldPosition( eAnchor const & anchor, UIWidget const * forChild = nullptr ) const override;
	bool IsHeld( ) const;

//-------------------------------------------------------------------------------------------------
// Events
//-------------------------------------------------------------------------------------------------
private:
	void OnItemPickup( NamedProperties & pickupEvent );
	void OnItemDrop( NamedProperties & dropEvent );
};