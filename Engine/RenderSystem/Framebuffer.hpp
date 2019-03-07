#pragma once

#include <vector>
#include "Engine/RenderSystem/Texture.hpp"


//-------------------------------------------------------------------------------------------------
class Framebuffer
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	Texture * m_depthStencilTarget;

public:
	unsigned int m_fboHandle; //GLuint
	unsigned int m_pixelWidth;
	unsigned int m_pixelHeight;
	std:: vector<Texture*> m_colorTargets;
	//#TODO: Go through entire codebase and add deletes to deconstructors with vectors of pointers

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Framebuffer( );
	Framebuffer( unsigned int width, unsigned int height, std::vector<TextureFormat> const & colorFormats, TextureFormat const & depthStencilFormat );
	~Framebuffer( );
	void Initialize( unsigned int width, unsigned int height, std::vector<TextureFormat> const & colorFormats, TextureFormat const & depthStencilFormat );
	unsigned int GetColorTexture( int colorIndex ) const;
	unsigned int GetDepthTexture( ) const;

	//#TODO:
	//Framebuffer* FramebufferCreate( unsigned int width, unsigned int height, std::vector<TextureFormat> const &colorFormats, TextureFormat const &depthStencilFormat );
};