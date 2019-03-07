#include "Engine/RenderSystem/Framebuffer.hpp"

#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>


//-------------------------------------------------------------------------------------------------
Framebuffer::Framebuffer( )
{
	std::vector<TextureFormat> colorFormats;
	colorFormats.push_back( TextureFormat_RGBA8 );
	Vector2i windowDimensions = GetWindowDimensions( );
	Initialize( windowDimensions.x, windowDimensions.y, colorFormats, TextureFormat_D24S8 );
}


//-------------------------------------------------------------------------------------------------
Framebuffer::Framebuffer( unsigned int width, unsigned int height, std::vector<TextureFormat> const & colorFormats, TextureFormat const & depthStencilFormat )
{
	Initialize( width, height, colorFormats, depthStencilFormat );
}


//-------------------------------------------------------------------------------------------------
Framebuffer::~Framebuffer( )
{
	for ( Texture *currentTex : m_colorTargets )
	{
		delete currentTex;
		currentTex = nullptr;
	}

	delete m_depthStencilTarget;
	m_depthStencilTarget = nullptr;
}


//-------------------------------------------------------------------------------------------------
void Framebuffer::Initialize( unsigned int width, unsigned int height, std::vector<TextureFormat> const & colorFormats, TextureFormat const & depthStencilFormat )
{
	std::vector<Texture*> colorTargets;
	for( unsigned int colorIndex = 0; colorIndex < colorFormats.size( ); ++colorIndex )
	{
		TextureFormat format = colorFormats[colorIndex];
		colorTargets.push_back( new Texture( width, height, format ) );
	}

	Texture * depthStencilTarget = new Texture( width, height, depthStencilFormat );

	//Create the Framebuffer Object
	unsigned int fboHandle;
	glGenFramebuffers( 1, &fboHandle );
	if( fboHandle == NULL )
	{
		ERROR_AND_DIE( "No handle created." )
	}

	// Set known values
	m_fboHandle = fboHandle;
	m_pixelWidth = width;
	m_pixelHeight = height;
	unsigned int colorCount = colorFormats.size( );

	//Set textures
	for( unsigned int currentColor = 0; currentColor < colorCount; ++currentColor )
	{
		m_colorTargets.push_back( colorTargets[currentColor] );
	}
	m_depthStencilTarget = depthStencilTarget;

	//OpenGL Initialization
	//If you bound a framebuffer to your Renderer - 
	//be careful you didn't unbind just now...
	glBindFramebuffer( GL_FRAMEBUFFER, fboHandle );

	// Bind our color targets to our FBO
	for( unsigned int i = 0; i < colorCount; ++i )
	{
		Texture *tex = colorTargets[i];
		glFramebufferTexture
		(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0 + i,
			tex->m_openglTextureID, //texture ID
			0
		);
	}

	// Bind depth stencil if it exists
	if( nullptr != depthStencilTarget )
	{
		glFramebufferTexture
		(
			GL_FRAMEBUFFER,
			GL_DEPTH_STENCIL_ATTACHMENT,
			depthStencilTarget->m_openglTextureID,
			0
		);
	}

	//Make sure everything was bound correctly
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE )
	{
		ERROR_AND_DIE( "Buffer creation failed." );
	}

	//Revert to old state
	glBindFramebuffer( GL_FRAMEBUFFER, NULL );

	//1. Create the textures
	//2. Then do steps below (FramebufferCreate)
	//3. Have FramebufferDelete also destroy the textures created
}


//-------------------------------------------------------------------------------------------------
unsigned int Framebuffer::GetColorTexture( int colorIndex ) const
{
	return m_colorTargets[colorIndex]->m_openglTextureID;
}


//-------------------------------------------------------------------------------------------------
unsigned int Framebuffer::GetDepthTexture() const
{
	return m_depthStencilTarget->m_openglTextureID;
}
