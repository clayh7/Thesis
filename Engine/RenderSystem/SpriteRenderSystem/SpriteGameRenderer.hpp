#pragma once
#include <string>
#include <map>
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"


//-------------------------------------------------------------------------------------------------
class Command;
class Framebuffer;
class Material;
class Mesh;
class MeshRenderer;
class SpriteGameRenderer;
class SpriteResource;
class SpriteSheetResource;
class SpriteLayer;
class Sprite;


//-------------------------------------------------------------------------------------------------
void EnableLayerCommand( Command const & command );
void DisableLayerCommand( Command const & command );
void ExportSpritesCommand( Command const & command );


//-------------------------------------------------------------------------------------------------
enum eMaterialEffect
{
	eMaterialEffect_NOTHING,
	eMaterialEffect_WAVES,
	eMaterialEffect_DARK,
	eMaterialEffect_COUNT,
};


//-------------------------------------------------------------------------------------------------
extern SpriteGameRenderer * g_SpriteRenderSystem;


//-------------------------------------------------------------------------------------------------
class SpriteGameRenderer
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * SPRITE_RESOURCE_NAME;
	static char const * SPRITE_SHEET_RESOURCE_NAME;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static Sprite * Create( std::string const & spriteID, int layer = 0, bool ignoreView = false );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	Vector2f m_virtualScreen;
	float m_virtualSize;
	float m_importSize;
	float m_aspectRatio;
	Color m_clearColor;
	Matrix4f m_viewMatrix;
	Matrix4f m_projectionMatrix;
	Mesh * m_spriteMesh;
	MeshRenderer * m_spriteMeshRenderer;

	std::map<size_t, SpriteResource*> m_spriteResourceDatabase;
	std::map<size_t, SpriteSheetResource*> m_spriteSheetResourceDatabase;
	mutable std::map<int, SpriteLayer*> m_spriteLayers; //so that CreateOrGetLayer can be const
	
	Framebuffer * m_fboCurrent;
	Framebuffer * m_fboEffect;
	Mesh const * m_screenMesh;
	std::vector<Material*> m_screenMaterials;
	MeshRenderer * m_screenEffect;
	mutable MeshBuilder m_tempBuilder; //Because it's used in ConstructMesh()

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	SpriteGameRenderer( );
	~SpriteGameRenderer( );

	void UpdateMaterialEffects( );
	void Render( ); //Not const because you have to swap FBOs (may make them mutable later)
	void RenderLayer( SpriteLayer const * layer ) const;
	void RenderSprite( Sprite const * sprite ) const;
	void SwapFBO( Framebuffer ** first, Framebuffer ** second );
	void SetupMaterialEffects( );
	void LoadAllSpriteResources( );
	void AddSpriteResource( std::string const & id, std::string const & filename );
	void AddSpriteResource( std::string const & id, std::string const & spriteSheetID, int spriteSheetIndex );
	void AddSpriteSheetResource( std::string const & spriteSheetID, std::string const & spriteSheetFilename, Vector2i const & spriteSheetSize );
	bool AddLayerEffect( int layer, eMaterialEffect const & effect );
	bool ClearLayerEffects( int layer );
	void ExportSpriteDatabase( std::string const & filename );

	SpriteLayer * CreateOrGetLayer( int layer ) const;
	SpriteResource const * GetSpriteResource( std::string const & spriteID ) const;
	SpriteSheetResource const * GetSpriteSheetResource( std::string const & spriteSheetID ) const;
	float GetVirtualSize( ) const;
	float GetImportSize( ) const;
	float GetAspectRatio( ) const;
	Vector2f GetVirtualScreen( ) const;
	Matrix4f GetView( ) const;
	bool IsSpriteOnScreen( Sprite const * sprite ) const;

	void ResetVirtualScreen( );
	void SetVirtualSize( float size );
	void SetImportSize( float size );
	void SetAspectRatio( float width, float height );
	void SetClearColor( Color const & color );
	bool SetLayerEnabled( int layer, bool isEnabled );
	void SetView( Matrix4f const & view );
};