//-----------------------------------------------------------------------------------------------
// StringUtils.cpp
// By: Squirrel Eiserloh

//-----------------------------------------------------------------------------------------------
#include <stdarg.h>
#include <algorithm>
#include <string>
#include <vector>
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	char* textLiteral = textLiteralSmall;
	if ( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[maxLength];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	textLiteral[maxLength - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if ( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//-------------------------------------------------------------------------------------------------
const std::string GetAsLowerCase( std::string const & mixedCaseString )
{
	std::string data = mixedCaseString;
	std::transform( data.begin( ), data.end( ), data.begin( ), ::tolower );
	return data;
}


//-------------------------------------------------------------------------------------------------
std::vector<std::string> SplitString( std::string const & rawString, char delimiter )
{
	std::vector<std::string> result;
	std::string workingString = rawString;
	size_t rangeIndex = workingString.find( delimiter );

	//Loop and parse each before delimiter
	while ( rangeIndex != std::string::npos )
	{
		result.push_back( workingString.substr( 0, rangeIndex ) );
		workingString = workingString.substr( rangeIndex + 1 );
		rangeIndex = workingString.find( delimiter );
	}

	//Add last part
	if ( workingString != "" )
	{
		result.push_back( workingString );
	}
	return result;
}


//-------------------------------------------------------------------------------------------------
std::vector<size_t> FindIndicies( std::string const & rawString, char delimiter )
{
	std::vector<size_t> result;
	std::string workingString = rawString;
	size_t delimiterIndex = workingString.find( delimiter );

	//Loop and parse each before delimiter
	while( delimiterIndex != std::string::npos )
	{
		result.push_back( delimiterIndex );
		workingString = workingString.substr( delimiterIndex + 1 );
		delimiterIndex = workingString.find( delimiter );
	}

	return result;
}


//-------------------------------------------------------------------------------------------------
bool AllUnique( std::string const & rawString )
{
	size_t const CHAR_COUNT = 256;
	if( rawString.size( ) > CHAR_COUNT )
	{
		return false;
	}

	bool uniqueCheck[CHAR_COUNT];
	for( size_t uniqueIndex = 0; uniqueIndex < CHAR_COUNT; ++uniqueIndex )
	{
		uniqueCheck[uniqueIndex] = false;
	}
	for( size_t charIndex = 0; charIndex < rawString.size(); ++charIndex )
	{
		char const & checkChar = rawString[charIndex];
		if( uniqueCheck[checkChar] )
		{
			return false;
		}
		uniqueCheck[checkChar] = true;
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
void Reverse( char * str )
{
	size_t MAX_LENGTH = 4092;
	size_t length = strnlen( str, MAX_LENGTH );
	if( length <= 1 )
	{
		return;
	}

	size_t startIndex = 0;
	size_t endIndex = length - 1;
	while( startIndex < endIndex )
	{
		char temp = str[startIndex];
		str[startIndex] = str[endIndex];
		str[endIndex] = temp;
		++startIndex;
		--endIndex;
	}
}


//-------------------------------------------------------------------------------------------------
char * CreateNewCString( std::string const & string )
{
	size_t newStringSize = string.size( ) + 1;
	char * newString = new char[newStringSize];
	strncpy_s( newString, newStringSize, string.c_str( ), newStringSize );
	return newString;
}


//-------------------------------------------------------------------------------------------------
void SortString( std::string & str )
{
	char * rawString = CreateNewCString( str );
	MergeSort( rawString, str.size( ) );
	str = rawString;
	delete rawString;
}


//-------------------------------------------------------------------------------------------------
bool IsPermutation( std::string const & str1, std::string const & str2 )
{
	//Make sure they're the same length
	size_t length = str1.size( );
	if( length != str1.size( ) )
	{
		return false;
	}
	
	//create char arrays for both strings
	char * sorted1 = (char*) _alloca( length );
	memcpy( sorted1, str1.c_str( ), length );
	char * sorted2 = (char*) _alloca( length );
	memcpy( sorted2, str2.c_str( ), length );

	//Sort the strings
	MergeSort( sorted1, length );
	MergeSort( sorted2, length );

	//Make sure they are equal
	for( size_t index = 0; index < length; ++index )
	{
		if( sorted1[index] != sorted2[index] )
		{
			return false;
		}
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
std::string CompressString( std::string const & str )
{
	std::string compressed;
	char charCurrent = str[0];
	size_t charCount = 1;
	size_t compressedLength = 0;
	size_t stringLength = str.size( );
	for( size_t index = 1; index < stringLength; ++index )
	{
		if( charCurrent != str[index] )
		{
			compressed.append( std::to_string( charCount ) );
			compressed.append( 1U, charCurrent );
			charCount = 1;
			charCurrent = str[index];
			compressedLength += 2;
		}
		else
		{
			++charCount;
		}
		if( compressedLength >= stringLength )
		{
			return str;
		}
	}
	return compressed;
}


//-------------------------------------------------------------------------------------------------
int ParseInt( std::string const &string )
{
	return std::stoi( string );
}