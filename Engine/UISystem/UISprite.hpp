#pragma once

#include "Engine/UISystem/UIWidget.hpp"


//-------------------------------------------------------------------------------------------------
class Mesh;
class Material;
class MeshRenderer;
class Transform;
class SpriteResource;
class UIWidgetRegistration;
struct XMLNode;


//-------------------------------------------------------------------------------------------------
class UISprite : public UIWidget
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static UIWidgetRegistration s_UISpriteRegistration;

public:
	static char const * DEFAULT_SPRITE_ID;
	static char const * PROPERTY_SPRITE_ID;
	static char const * PROPERTY_ROTATION_DEGREES;

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
	UISprite( std::string const & name = "" );
	UISprite( XMLNode const & node );
	virtual ~UISprite( ) override;

	virtual void Update( ) override;
	virtual void Render( ) const override;

protected:
	void SetupRenderers( );
	void UpdateRenderers( );
	void CreateSpriteMesh( Mesh * out_mesh );
	Transform CalcQuadTransform( );
	void PopulateFromXML( XMLNode const & node );
};