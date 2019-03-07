#pragma once

#include <string>
#include <map>
#include "Engine/Math/Vector2i.hpp"
#include "Engine/Memory/MemoryAnalytics.hpp"


//-------------------------------------------------------------------------------------------------
typedef unsigned int GLuint;


//-------------------------------------------------------------------------------------------------
enum TextureFormat
{
	TextureFormat_RGBA8, //RGBA, 8 bits per channel
	TextureFormat_D24S8, //Depth 24, Stencil 8
};


//-------------------------------------------------------------------------------------------------
class Texture
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	//Static Memory map allocation
	static std::map<size_t, Texture*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, Texture*>>> s_textureRegistry;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static Texture const * CreateOrLoadTexture( std::string const & imageFilePath );
	static void DestroyRegistry( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	GLuint m_openglTextureID;
	Vector2i m_texelSize;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
private:
	Texture( std::string const & imageFilePath );

public:
	Texture( unsigned int width, unsigned int height, TextureFormat const & format );
	~Texture( );

	Vector2i GetTexelSize( ) const { return m_texelSize; };
};