#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"

#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/RenderSystem/Framebuffer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteResource.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteSheetResource.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteLayer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
void EnableLayerCommand( Command const & command )
{
	int layer = command.GetArg( 0, 0 );
	if( g_SpriteRenderSystem->SetLayerEnabled( layer, true ) )
	{
		g_ConsoleSystem->AddLog( Stringf( "Layer %d enabled", layer ), Console::GOOD );
	}
	else
	{
		g_ConsoleSystem->AddLog( Stringf( "Layer %d does not exist", layer ), Console::BAD );
	}
}


//-------------------------------------------------------------------------------------------------
void DisableLayerCommand( Command const & command )
{
	int layer = command.GetArg( 0, 0 );
	if( g_SpriteRenderSystem->SetLayerEnabled( layer, false ) )
	{
		g_ConsoleSystem->AddLog( Stringf( "Layer %d disabled", layer ), Console::GOOD );
	}
	else
	{
		g_ConsoleSystem->AddLog( Stringf( "Layer %d does not exist", layer ), Console::BAD );
	}
}


//-------------------------------------------------------------------------------------------------
void ExportSpritesCommand( Command const & command )
{
	std::string filename = command.GetArg( 0, "SpriteDatabase" );
	g_SpriteRenderSystem->ExportSpriteDatabase( filename );
	g_ConsoleSystem->AddLog( Stringf( "Exported sprite resource database to %s.Sprite.xml", filename.c_str() ), Console::GOOD );
}


//-------------------------------------------------------------------------------------------------
STATIC SpriteGameRenderer * g_SpriteRenderSystem = nullptr;
STATIC char const * SpriteGameRenderer::SPRITE_RESOURCE_NAME = "SpriteResource";
STATIC char const * SpriteGameRenderer::SPRITE_SHEET_RESOURCE_NAME = "SpriteSheetResource";


//-------------------------------------------------------------------------------------------------
//Higher layer on top
Sprite * SpriteGameRenderer::Create( std::string const & spriteID, int layer /*= 0*/, bool ignoreView /*= false*/ )
{
	Sprite * newSprite = new Sprite( spriteID, layer, ignoreView );
	SpriteLayer * spriteLayer = g_SpriteRenderSystem->CreateOrGetLayer( layer );
	spriteLayer->AddSprite( newSprite );
	return newSprite;
}


//-------------------------------------------------------------------------------------------------
SpriteGameRenderer::SpriteGameRenderer( )
	: m_fboCurrent( nullptr )
	, m_fboEffect( nullptr )
	, m_screenMesh( nullptr )
	, m_screenEffect( nullptr )
	, m_virtualScreen( 1.f, 1.f )
	, m_virtualSize( 1.f )
	, m_importSize( 1.f )
	, m_aspectRatio( 1.f )
	, m_clearColor( Color::BLACK )
	, m_viewMatrix( Matrix4f::IDENTITY )
	, m_projectionMatrix( Matrix4f::IDENTITY )
	, m_spriteMesh( nullptr )
	, m_spriteMeshRenderer( nullptr )
	, m_tempBuilder( )
{
	LoadAllSpriteResources( );
	SetupMaterialEffects( );
	m_spriteMesh = new Mesh( eVertexType_SPRITE );
	m_spriteMeshRenderer = new MeshRenderer( Transform(), RenderState::BASIC_2D );

	//Create Nothing Shader PostProcessFX
	m_fboCurrent = new Framebuffer( );
	m_fboEffect = new Framebuffer( );
	m_screenMesh = Mesh::GetMeshShape( eMeshShape_QUAD );
	m_screenEffect = new MeshRenderer( m_screenMesh, m_screenMaterials[eMaterialEffect_NOTHING], Transform( Vector3f::ZERO, Matrix4f::IDENTITY, 2.f ), RenderState::BASIC_2D );

	g_ConsoleSystem->RegisterCommand( "sprite_layer_enable", EnableLayerCommand, " [num] : Enables sprite layer, allowing it to render. Default = 0" );
	g_ConsoleSystem->RegisterCommand( "sprite_layer_disable", DisableLayerCommand, " [num] : Disable sprite layer, stopping it from rendering. Default = 0" );
	g_ConsoleSystem->RegisterCommand( "sprite_export", ExportSpritesCommand, " [filename] : Save sprite resource database to an xml file. Delfault = SpriteDatabase" );
}


//-------------------------------------------------------------------------------------------------
SpriteGameRenderer::~SpriteGameRenderer( )
{
	delete m_spriteMesh;
	m_spriteMesh = nullptr;

	delete m_spriteMeshRenderer;
	m_spriteMeshRenderer = nullptr;
	
	delete m_screenEffect;
	m_screenEffect = nullptr;

	for( Material * deleteMaterial : m_screenMaterials )
	{
		delete deleteMaterial;
		deleteMaterial = nullptr;
	}
	m_screenMaterials.clear( );

	delete m_fboCurrent;
	m_fboCurrent = nullptr;

	delete m_fboEffect;
	m_fboEffect = nullptr;

	for( auto deleteResource : m_spriteResourceDatabase )
	{
		delete deleteResource.second;
		deleteResource.second = nullptr;
	}
	m_spriteResourceDatabase.clear( );

	for( auto deleteResource : m_spriteSheetResourceDatabase )
	{
		delete deleteResource.second;
		deleteResource.second = nullptr;
	}
	m_spriteSheetResourceDatabase.clear( );

	for( auto deleteLayer : m_spriteLayers )
	{
		delete deleteLayer.second;
		deleteLayer.second = nullptr;
	}
	m_spriteLayers.clear( );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::UpdateMaterialEffects( )
{
	m_screenMaterials[eMaterialEffect_WAVES]->SetUniform( "uTime", Time::TOTAL_SECONDS );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::Render( )
{
	//Save active FBO
	Framebuffer * startingFBO = g_RenderSystem->GetActiveFBO( );

	//Setup current FBO
	g_RenderSystem->BindFramebuffer( m_fboCurrent );
	g_RenderSystem->ClearScreen( m_clearColor );

	//Render Sprite layers
	for( auto spriteLayerIter = m_spriteLayers.begin( ); spriteLayerIter != m_spriteLayers.end(); ++spriteLayerIter )
	{
		//Render layer of sprites
		SpriteLayer * currentLayer = spriteLayerIter->second;
		RenderLayer( currentLayer );

		//Apply FBO effects
		for( eMaterialEffect const & effectID : currentLayer->m_effects )
		{
			g_RenderSystem->BindFramebuffer( m_fboEffect );
			m_screenEffect->SetMaterial( m_screenMaterials[effectID] );
			m_screenEffect->SetUniform( "uDiffuseTex", m_fboCurrent->GetColorTexture( 0 ) );
			m_screenEffect->Render( );
			SwapFBO( &m_fboCurrent, &m_fboEffect );
		}
	}

	//Redraw game onto starting FBO
	g_RenderSystem->BindFramebuffer( startingFBO );
	m_screenEffect->SetMaterial( m_screenMaterials[eMaterialEffect_NOTHING] );
	m_screenEffect->SetUniform( "uDiffuseTex", m_fboCurrent->GetColorTexture( 0 ) );
	m_screenEffect->Render( );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::RenderLayer( SpriteLayer const * layer ) const
{
	//Only draw enabled layers
	if( !layer->IsEnabled( ) )
	{
		return;
	}

	//Traverse sprite list and draw each one to screen
	Sprite * currentSprite = layer->m_listStart;
	while( currentSprite != nullptr )
	{
		RenderSprite( currentSprite );
		currentSprite = currentSprite->m_nextSprite;
	}
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::RenderSprite( Sprite const * sprite ) const
{
	if( !sprite->IsEnabled( ) )
	{
		return;
	}

	if( !IsSpriteOnScreen( sprite ) )
	{
		return;
	}

	//Setup Mesh
	sprite->ConstructMesh( m_spriteMesh, m_tempBuilder );

	//Setup Material
	Material * spriteMaterial = sprite->GetMaterial( );

	if( sprite->m_ignoreView )
	{
		spriteMaterial->SetUniform( "uView", Matrix4f::IDENTITY );
	}
	else
	{
		spriteMaterial->SetUniform( "uView", GetView( ) );
	}
	spriteMaterial->SetUniform( "uProj", m_projectionMatrix );
	spriteMaterial->SetUniform( "uTexDiffuse", sprite->GetTexture( ) );

	//Setup MeshRenderer
	m_spriteMeshRenderer->SetMeshAndMaterial( m_spriteMesh, spriteMaterial );

	//Draw
	m_spriteMeshRenderer->Render( );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::SwapFBO( Framebuffer ** out_first, Framebuffer ** out_second )
{
	Framebuffer * temp = *out_first;
	*out_first = *out_second;
	*out_second = temp;
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::SetupMaterialEffects( )
{
	m_screenMaterials.resize( eMaterialEffect_COUNT );
	m_screenMaterials[eMaterialEffect_NOTHING] = new Material( "Data/Shaders/post.vert", "Data/Shaders/postNothing.frag" );
	m_screenMaterials[eMaterialEffect_WAVES] = new Material( "Data/Shaders/post.vert", "Data/Shaders/postWaves.frag" );
	m_screenMaterials[eMaterialEffect_DARK] = new Material( "Data/Shaders/post.vert", "Data/Shaders/postDark.frag" );
	ASSERT_RECOVERABLE( m_screenMaterials.size( ) == eMaterialEffect_COUNT, "Material Effect not added" );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::LoadAllSpriteResources( )
{
	std::vector<std::string> spriteFiles = EnumerateFilesInFolder( "Data/Sprites/", "*.Sprite.xml" );
	for( std::string const & spriteFile : spriteFiles )
	{
		XMLNode spriteFileNode = XMLNode::openFileHelper( spriteFile.c_str() ).getChildNode(0);

		//Load all the sprite sheets
		for( int spriteIndex = 0; spriteIndex < spriteFileNode.nChildNode( ); ++spriteIndex )
		{
			XMLNode spriteData = spriteFileNode.getChildNode( spriteIndex );
			std::string name = spriteData.getName( );
			if( name == SPRITE_SHEET_RESOURCE_NAME )
			{
				std::string spriteSheetID = ReadXMLAttribute( spriteData, "id", "" );
				std::string spriteSheetFilename = ReadXMLAttribute( spriteData, "filename", "error" );
				Vector2i spriteSheetSize = ReadXMLAttribute( spriteData, "size", Vector2i( 1, 1 ) );
				AddSpriteSheetResource( spriteSheetID, spriteSheetFilename, spriteSheetSize );
			}
		}

		//Load all the sprite resources
		for( int spriteIndex = 0; spriteIndex < spriteFileNode.nChildNode( ); ++spriteIndex )
		{
			XMLNode spriteData = spriteFileNode.getChildNode( spriteIndex );
			std::string name = spriteData.getName( );
			if( name == SPRITE_RESOURCE_NAME )
			{
				std::string spriteID = ReadXMLAttribute(spriteData, "id", "" );
				std::string spriteFilename = ReadXMLAttribute( spriteData, "filename", "error" );
				//Must be a sprite from a sprite sheet
				if( spriteFilename == "error" )
				{
					int spriteSheetIndex = ReadXMLAttribute( spriteData, "index", 0 );
					std::string spriteSheetID = ReadXMLAttribute( spriteData, "spriteSheet", "error" );
					AddSpriteResource( spriteID, spriteSheetID, spriteSheetIndex );
				}
				else
				{
					AddSpriteResource( spriteID, spriteFilename );
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::AddSpriteResource( std::string const & id, std::string const & filename )
{
	size_t idHash = std::hash<std::string>{ }( id );
	auto foundResource = m_spriteResourceDatabase.find( idHash );

	//Does not exist yet
	if( foundResource == m_spriteResourceDatabase.end( ) )
	{
		SpriteResource * spriteResource = new SpriteResource( id, filename );
		m_spriteResourceDatabase.insert( std::pair<size_t, SpriteResource*>( idHash, spriteResource ) );
	}
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::AddSpriteResource( std::string const & id, std::string const & spriteSheetID, int spriteSheetIndex )
{
	size_t idHash = std::hash<std::string>{ }( id );
	auto foundResource = m_spriteResourceDatabase.find( idHash );

	//Does not exist yet
	if( foundResource == m_spriteResourceDatabase.end( ) )
	{
		SpriteSheetResource const * spriteSheetResource = GetSpriteSheetResource( spriteSheetID );
		SpriteResource * spriteResource = new SpriteResource( id, spriteSheetResource, spriteSheetIndex );
		m_spriteResourceDatabase.insert( std::pair<size_t, SpriteResource*>( idHash, spriteResource ) );
	}
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::AddSpriteSheetResource( std::string const & spriteSheetID, std::string const & spriteSheetFilename, Vector2i const & spriteSheetSize )
{
	size_t idHash = std::hash<std::string>{ }( spriteSheetID );
	auto foundResource = m_spriteSheetResourceDatabase.find( idHash );

	//Does not exist yet
	if( foundResource == m_spriteSheetResourceDatabase.end( ) )
	{
		SpriteSheetResource * spriteSheetResource = new SpriteSheetResource( spriteSheetID, spriteSheetFilename, spriteSheetSize );
		m_spriteSheetResourceDatabase.insert( std::pair<size_t, SpriteSheetResource*>( idHash, spriteSheetResource ) );
	}
}


//-------------------------------------------------------------------------------------------------
bool SpriteGameRenderer::AddLayerEffect( int layer, eMaterialEffect const & effect )
{
	//Return true if setting the layer effect was successful
	auto foundLayer = m_spriteLayers.find( layer );
	if( foundLayer != m_spriteLayers.end( ) )
	{
		foundLayer->second->AddEffect( effect );
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
bool SpriteGameRenderer::ClearLayerEffects( int layer )
{
	//Return true if clearing the layer effects was successful
	auto foundLayer = m_spriteLayers.find( layer );
	if( foundLayer != m_spriteLayers.end( ) )
	{
		foundLayer->second->RemoveAllEffects( );
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::ExportSpriteDatabase( std::string const & filename )
{
	XMLNode databaseRoot = XMLNode::createXMLTopNode( "SpriteResources" );
	std::string outFile = Stringf( "Data/Sprites/%s.Sprite.xml", filename.c_str( ) );
	for( auto spriteResourceIter : m_spriteResourceDatabase )
	{
		SpriteResource * resource = spriteResourceIter.second;
		XMLNode resourceNode = databaseRoot.addChild( "SpriteResource" );
		resourceNode.addAttribute( "id", resource->m_id );
		resourceNode.addAttribute( "filename", resource->m_filename );
	}
	databaseRoot.writeToFile( outFile.c_str( ) );
}


//-------------------------------------------------------------------------------------------------
SpriteLayer * SpriteGameRenderer::CreateOrGetLayer( int layer ) const
{
	auto layerFound = m_spriteLayers.find( layer );
	if( layerFound != m_spriteLayers.end( ) )
	{
		return layerFound->second;
	}
	else
	{
		SpriteLayer * spriteLayer = new SpriteLayer( layer );
		m_spriteLayers.insert( std::pair<int, SpriteLayer*>( layer, spriteLayer ) );
		return spriteLayer;
	}
}


//-------------------------------------------------------------------------------------------------
SpriteResource const * SpriteGameRenderer::GetSpriteResource( std::string const & spriteID ) const
{
	size_t spriteIDHash = std::hash<std::string>{ }( spriteID );
	auto spriteResourceFound = m_spriteResourceDatabase.find( spriteIDHash );
	if( spriteResourceFound == m_spriteResourceDatabase.end( ) )
	{
		ERROR_AND_DIE( "Sprite resource not found." );
	}
	return spriteResourceFound->second;
}


//-------------------------------------------------------------------------------------------------
SpriteSheetResource const * SpriteGameRenderer::GetSpriteSheetResource( std::string const & spriteSheetID ) const
{
	size_t spriteSheetIDHash = std::hash<std::string>{ }( spriteSheetID );
	auto spriteSheetResourceFound = m_spriteSheetResourceDatabase.find( spriteSheetIDHash );
	if( spriteSheetResourceFound == m_spriteSheetResourceDatabase.end( ) )
	{
		ERROR_AND_DIE( "Sprite Sheet resource not found." );
	}
	return spriteSheetResourceFound->second;
}


//-------------------------------------------------------------------------------------------------
float SpriteGameRenderer::GetVirtualSize( ) const
{
	return m_virtualSize;
}


//-------------------------------------------------------------------------------------------------
float SpriteGameRenderer::GetImportSize( ) const
{
	return m_importSize;
}


//-------------------------------------------------------------------------------------------------
float SpriteGameRenderer::GetAspectRatio( ) const
{
	return m_aspectRatio;
}


//-------------------------------------------------------------------------------------------------
Vector2f SpriteGameRenderer::GetVirtualScreen( ) const
{
	return m_virtualScreen;
}


//-------------------------------------------------------------------------------------------------
Matrix4f SpriteGameRenderer::GetView( ) const
{
	return m_viewMatrix;
}


//-------------------------------------------------------------------------------------------------
bool SpriteGameRenderer::IsSpriteOnScreen( Sprite const * sprite ) const
{
	//If you ignore the view matrix, never cull!
	if( sprite->m_ignoreView )
	{
		return true;
	}

	Vector2f camPosition = m_viewMatrix.GetWorldPositionFromViewNoScale( ).XY( );
	float spriteDistance = DistanceBetweenPoints( sprite->m_position, camPosition );
	float maxDistance = m_virtualSize + sprite->GetRadiusScaled( );

	return spriteDistance < maxDistance;
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::ResetVirtualScreen( )
{
	if( m_aspectRatio > 1.f )
	{
		m_virtualScreen.x = m_virtualSize * m_aspectRatio;
		m_virtualScreen.y = m_virtualSize;
	}
	else
	{
		m_virtualScreen.x = m_virtualSize;
		m_virtualScreen.y = m_virtualSize / m_aspectRatio;
	}
	m_projectionMatrix.MakeOrthonormal( m_virtualScreen.x, m_virtualScreen.y, -1, 1 );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::SetVirtualSize( float size )
{
	m_virtualSize = size;
	ResetVirtualScreen( );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::SetImportSize( float size )
{
	m_importSize = size;
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::SetAspectRatio( float width, float height )
{
	m_aspectRatio = width / height;
	ResetVirtualScreen( );
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::SetClearColor( Color const & color )
{
	m_clearColor = color;
}


//-------------------------------------------------------------------------------------------------
bool SpriteGameRenderer::SetLayerEnabled( int layer, bool isEnabled )
{
	//Return true if setting layer was successful
	auto foundLayer = m_spriteLayers.find( layer );
	if( foundLayer != m_spriteLayers.end( ) )
	{
		foundLayer->second->SetEnabled( isEnabled );
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
void SpriteGameRenderer::SetView( Matrix4f const & view )
{
	m_viewMatrix = view;
}