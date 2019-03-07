#include "Engine/RenderSystem/BitmapFont.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/RenderSystem/Glyph.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/Kerning.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>

#define STBI_HEADER_FILE_ONLY
#include "ThirdParty/stb/stb_image.c"


//-------------------------------------------------------------------------------------------------
STATIC std::map<size_t, BitmapFont*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, BitmapFont*>>> BitmapFont::s_fontRegistry;
STATIC Kerning const BitmapFont::DOES_NOT_EXIST = Kerning( 0, 0, 0 );


//-------------------------------------------------------------------------------------------------
std::string CutToNthOccurence( std::string const & stringToParse, int num, std::string const & delimeter )
{
	std::size_t dataIndex = stringToParse.find( delimeter );
	std::string cutString = stringToParse.substr( dataIndex + 1 );
	for( int lineCount = 0; lineCount < num; ++lineCount )
	{
		std::size_t dataIndex = cutString.find( delimeter );
		cutString = cutString.substr( dataIndex + 1 );
	}
	return cutString;
}


//-------------------------------------------------------------------------------------------------
BitmapFont::BitmapFont( std::string const & bitmapFontInfo, bool parseFNT /*= true*/ )
	: m_openglTextureID( 0 )
	, m_texelSize( 0, 0 )
	, m_lineHeight( 0 )
	, m_base( 0 )
{
	m_glyphs.resize( 256 );

	//Not Parsing .fnt file
	if ( !parseFNT ) //basically means this is mono-spaced font
	{
		GenerateTexture( bitmapFontInfo );

		//Figuring out size of mono-spaced font
		m_lineHeight = m_texelSize.x / 8;
		m_base = m_texelSize.x / 8;
		m_size = m_texelSize.x / 8;

		int glyphCount = 256;
		Vector2i spriteSheetSize( 16, 16 ); //Default mono-spaced fonts are 16x16
		int glyphWidth = m_texelSize.x / spriteSheetSize.x;
		int glyphHeight = m_texelSize.y / spriteSheetSize.y;

		//Building Glyph List
		for ( int glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex )
		{
			int glyphX = ( glyphIndex % spriteSheetSize.x )*glyphWidth;
			int glyphY = ( glyphIndex / spriteSheetSize.x ) * glyphHeight;
			m_glyphs[( unsigned char ) glyphIndex] = new Glyph( glyphIndex, glyphX, glyphY, glyphWidth, glyphHeight, 0, 0, glyphWidth, m_texelSize );
		}
	}
	//Parsing .fnt file
	else
	{
		//Check location of the .fnt file
		std::string fileData;
		bool loaded = LoadBinaryFileToBuffer( bitmapFontInfo, fileData );
		ASSERT_RECOVERABLE( loaded, "Invalid (.fnt) file. Check the name, and it's location." );

		//Location of font image
		std::string bitmapFontName = GetString( fileData, "file=" );
		std::string bitmapFontPath = Stringf( "Data/Fonts/%s", &bitmapFontName[0] );
		GenerateTexture( bitmapFontPath );

		//Get Font Data
		m_lineHeight = GetInt( fileData, "lineHeight=" );
		m_base = GetInt( fileData, "base=" );
		m_size = GetInt( fileData, "size=" );

		//Clear glyph list
		for ( unsigned char glyphIndex = 0; glyphIndex < 255; ++glyphIndex )
		{
			m_glyphs[glyphIndex] = nullptr;
		}

		//Building Glyph List
		int glyphCount = GetInt( fileData, "chars count=" );
		for ( int glyphLine = 0; glyphLine < glyphCount; ++glyphLine )
		{
			std::string charInfoString = CutToNthOccurence( fileData, glyphLine, "char id=" );
			int glyphID = GetInt( charInfoString, "id=" );
			int glyphX = GetInt( charInfoString, "x=" );
			int glyphY = GetInt( charInfoString, "y=" );
			int glyphWidth = GetInt( charInfoString, "width=" );
			int glyphHeight = GetInt( charInfoString, "height=" );
			int glyphXOffset = GetInt( charInfoString, "xoffset=" );
			int glyphYOffset = GetInt( charInfoString, "yoffset=" );
			int glyphXadvance = GetInt( charInfoString, "xadvance=" );

			m_glyphs[( unsigned char ) glyphID] = new Glyph( glyphID, glyphX, glyphY, glyphWidth, glyphHeight, glyphXOffset, glyphYOffset, glyphXadvance, m_texelSize );
		}

		//Building Kerning List
		int kerningCount = GetInt( fileData, "kernings count=" );
		for ( int kerningLine = 0; kerningLine < kerningCount; ++kerningLine )
		{
			std::string kerningInfoString = CutToNthOccurence( fileData, kerningLine, "kerning " ); //space after 'kerning ' is important, so it doesn't find 'kernings'
			unsigned int kerningFirst = ( unsigned int ) GetInt( kerningInfoString, "first=" );
			unsigned int kerningSecond = ( unsigned int ) GetInt( kerningInfoString, "second=" );
			int kerningAmount = GetInt( kerningInfoString, "amount=" );

			//I'm storing both first and second indexs in a single short for it's index
			// 0000 -FIRST
			// 1111 -SECOND
			// index = 0000 1111
			unsigned short kerningCompactIndex = ( unsigned short ) kerningFirst;
			kerningCompactIndex = kerningCompactIndex << 4;
			kerningCompactIndex |= kerningSecond;
			
			auto kerningFound = m_kernings.find( kerningCompactIndex );
			if( kerningFound == m_kernings.end( ) )
			{
				m_kernings[kerningCompactIndex] = new Kerning( kerningFirst, kerningSecond, kerningAmount );
			}
			else
			{
				//#TODO: Duplicate Kernings, figure out why (I'm doing something wrong)
				delete m_kernings[kerningCompactIndex];
				m_kernings[kerningCompactIndex] = new Kerning( kerningFirst, kerningSecond, kerningAmount );
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
BitmapFont::~BitmapFont( )
{
	for ( Glyph * deleteGlyph : m_glyphs )
	{
		delete deleteGlyph;
		deleteGlyph = nullptr;
	}
	m_glyphs.clear( );

	for ( auto deleteKerning : m_kernings )
	{
		delete deleteKerning.second;
		deleteKerning.second = nullptr;
	}
	m_kernings.clear( );
}


//-------------------------------------------------------------------------------------------------
int BitmapFont::GetInt( std::string const & stringToParse, std::string const & delimeter )
{
	std::size_t dataIndex = stringToParse.find( delimeter );
	std::string cutString = stringToParse.substr( dataIndex );
	std::size_t startIndex = cutString.find( "=" ) + 1;
	std::size_t endIndex = cutString.find( " " );
	int numberLength = endIndex - startIndex;
	std::string number = cutString.substr( startIndex, numberLength );

	//Convert string to int and return
	return atoi( number.c_str( ) );
}


//-------------------------------------------------------------------------------------------------
std::string BitmapFont::GetString( std::string const & stringToParse, std::string const & delimeter )
{
	std::size_t dataIndex = stringToParse.find( delimeter );
	std::string cutString = stringToParse.substr( dataIndex );
	std::size_t startIndex = cutString.find( "\"" ) + 1;
	cutString = cutString.substr( startIndex );
	std::size_t endIndex = cutString.find( "\"" );

	return cutString.substr( 0, endIndex );
}


//-------------------------------------------------------------------------------------------------
BitmapFont* BitmapFont::CreateOrGetFont( std::string const & bitmapFontName )
{
	size_t nameHash = std::hash<std::string>{ }( bitmapFontName );
	auto foundFontIter = s_fontRegistry.find( nameHash );
	if ( foundFontIter != s_fontRegistry.end( ) )
		return foundFontIter->second;

	std::string delimiter = ".";
	std::string token = bitmapFontName.substr( bitmapFontName.find( delimiter ), bitmapFontName.length() );
	//Is the file type .fnt?
	bool parse = ( token == ".fnt" );
	BitmapFont* newFont = new BitmapFont( bitmapFontName, parse );
	s_fontRegistry[nameHash] = newFont;
	return newFont;
}


//-------------------------------------------------------------------------------------------------
void BitmapFont::DestroyRegistry( )
{
	for ( auto deleteFont : s_fontRegistry )
	{
		delete deleteFont.second;
		deleteFont.second = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
void BitmapFont::GenerateTexture( std::string const & bitmapFontPath )
{
	int numComponents = 0;
	unsigned char* imageData = stbi_load( bitmapFontPath.c_str( ), &m_texelSize.x, &m_texelSize.y, &numComponents, 0 );
	ASSERT_RECOVERABLE( numComponents == 4, "Invalid image format. Currently only supporting (.fnt) with 4 component." );

	// Enable texturing
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glGenTextures( 1, (GLuint*) &m_openglTextureID );
	glBindTexture( GL_TEXTURE_2D, m_openglTextureID );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_texelSize.x, m_texelSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData );
	stbi_image_free( imageData );
}


//-------------------------------------------------------------------------------------------------
const unsigned int BitmapFont::GetTextureID( ) const
{
	return m_openglTextureID;
}


//-------------------------------------------------------------------------------------------------
int BitmapFont::GetTextureWidth( ) const
{
	return m_texelSize.x;
}


//-------------------------------------------------------------------------------------------------
int BitmapFont::GetTextureHeight( ) const
{
	return m_texelSize.y;
}


//-------------------------------------------------------------------------------------------------
Glyph const * BitmapFont::GetGlyph( unsigned char glyphToDraw ) const
{
	return m_glyphs[glyphToDraw];
}


//-------------------------------------------------------------------------------------------------
Kerning const & BitmapFont::GetKerning( Glyph const * previousGlyph, Glyph const * glyph ) const
{
	//If a glyph doesn't exist, return nothing
	if ( previousGlyph == nullptr || glyph == nullptr )
	{
		return DOES_NOT_EXIST;
	}

	//Converting the two chars to be a short since it's compacted for its index
	unsigned char firstID = ( unsigned char ) previousGlyph->m_id;
	unsigned char secondID = ( unsigned char ) glyph->m_id;
	unsigned short kerningCompactIndex = ( unsigned short ) firstID;
	kerningCompactIndex = kerningCompactIndex << 4;
	kerningCompactIndex |= secondID;
	auto foundFontIter = m_kernings.find( kerningCompactIndex );
	if ( foundFontIter != m_kernings.end( ) )
	{
		return *foundFontIter->second;
	}

	//default
	return DOES_NOT_EXIST;
}


//-------------------------------------------------------------------------------------------------
int BitmapFont::GetFontSize( ) const
{
	return m_size;
}