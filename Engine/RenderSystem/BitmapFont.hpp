#pragma once

#include <string>
#include <map>
#include <vector>
#include "Engine/Math/Vector2i.hpp"
#include "Engine/Memory/MemoryAnalytics.hpp"


//-------------------------------------------------------------------------------------------------
class Glyph;
class Kerning;


//-------------------------------------------------------------------------------------------------
class BitmapFont
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	//Static memory allocation
	static std::map<size_t, BitmapFont*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, BitmapFont*>>> s_fontRegistry;
	static Kerning const DOES_NOT_EXIST;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static BitmapFont * CreateOrGetFont( std::string const & bitmapFontName );
	static void DestroyRegistry( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	int m_openglTextureID;
	Vector2i m_texelSize;
	int m_lineHeight;
	int m_base;
	int m_size;

	std::vector<Glyph*> m_glyphs;
	std::map<unsigned short, Kerning*> m_kernings;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
private:
	BitmapFont( std::string const & bitmapFontInfo, bool parseFNT = true );
	~BitmapFont( );
	int GetInt( std::string const & stringToParse, std::string const & delimeter );
	std::string GetString( std::string const & stringToParse, std::string const & delimeter );

public:
	void GenerateTexture( std::string const & bitmapFontPath );

	const unsigned int GetTextureID( ) const;
	int GetTextureWidth( ) const;
	int GetTextureHeight( ) const;
	Glyph const * GetGlyph( unsigned char glyphToDraw ) const;
	Kerning const & GetKerning( Glyph const * previousGlyph, Glyph const * glyph ) const;
	int GetFontSize( ) const;
};