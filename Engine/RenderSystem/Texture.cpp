//---------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
#include "Engine/RenderSystem/Texture.hpp"

#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Utils/StringUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>

#define STBI_HEADER_FILE_ONLY
#include "ThirdParty/stb/stb_image.c"

#define STATIC // Do-nothing indicator that method/member is static in class definition


//---------------------------------------------------------------------------
STATIC std::map<size_t, Texture*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, Texture*>>> Texture::s_textureRegistry;


//-------------------------------------------------------------------------------------------------
// Finds the named Texture among the registry of those already loaded
// If found, returns that Texture*.
// If not, attempts to load that texture, and returns the Texture* just created.
STATIC Texture const * Texture::CreateOrLoadTexture( std::string const & imageFilePath )
{
	size_t textureHash = std::hash<std::string>{ }( imageFilePath );
	auto found = s_textureRegistry.find( textureHash );
	if( found != s_textureRegistry.end( ) )
	{
		return found->second;
	}
	else
	{
		s_textureRegistry[textureHash] = new Texture( imageFilePath );
		return s_textureRegistry[textureHash];
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void Texture::DestroyRegistry( )
{
	for( auto shaderIndex = s_textureRegistry.begin( ); shaderIndex != s_textureRegistry.end( ); ++shaderIndex )
	{
		delete shaderIndex->second;
		shaderIndex->second = nullptr;
	}
}


//---------------------------------------------------------------------------
//#TODO: Refactor? Or just remove the things that are not needed because we are using shaders now!
Texture::Texture( std::string const & imageFilePath )
: m_openglTextureID( 0 )
, m_texelSize( 0, 0 )
{
	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)
	unsigned char* imageData = stbi_load( imageFilePath.c_str( ), &m_texelSize.x, &m_texelSize.y, &numComponents, numComponentsRequested );

	// Enable texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, ( GLuint* )&m_openglTextureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, m_openglTextureID );

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP ); // one of: GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR

	//Last is GL_LINEAR FOR MIPS
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if ( numComponents == 3 )
		bufferFormat = GL_RGB;

	// #FIXME: What happens if numComponents is neither 3 nor 4?
	bool checkComponents = (numComponents == 3) || (numComponents == 4);
	ASSERT_RECOVERABLE(  checkComponents, Stringf("Error loading Texture: Number of components (%i) per texel on loaded texture is not either 3(RGB) or 4(RGBA).",numComponents) );

	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		m_texelSize.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		m_texelSize.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageData );		// Location of the actual pixel data bytes/buffer

	stbi_image_free( imageData );

	// Disable texturing
	//RenderSystem->UnbindTexture( );
}


//-------------------------------------------------------------------------------------------------
Texture::Texture( unsigned int width, unsigned int height, TextureFormat const &format )
	: m_openglTextureID( 0 )
	, m_texelSize( 0, 0 )
{
	glGenTextures( 1, &m_openglTextureID );

	GLenum bufferChannels = GL_RGBA;
	GLenum bufferFormat = GL_UNSIGNED_INT_8_8_8_8;
	GLenum internalFormat = GL_RGBA8;

	if ( format == TextureFormat_RGBA8 )
	{
		//Hey look, nothing
	}
	else if ( format == TextureFormat_D24S8 )
	{
		bufferChannels = GL_DEPTH_STENCIL;
		bufferFormat = GL_UNSIGNED_INT_24_8;
		internalFormat = GL_DEPTH24_STENCIL8;
	}

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_openglTextureID );
	glTexImage2D( GL_TEXTURE_2D,
		0, //Mipmap level
		internalFormat, //How texture is stored in memory
		width,
		height,
		0, //border, again set to 0, we want not 0
		bufferChannels, //channels used by image being passed in
		bufferFormat, // format of data of image passed in
		NULL // No actual data passed in, (defaults to black)??
		);

	m_texelSize.x = width;
	m_texelSize.y = height;
}


//-------------------------------------------------------------------------------------------------
Texture::~Texture( )
{
	//Nothing
}