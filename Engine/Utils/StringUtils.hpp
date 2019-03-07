#pragma once

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
std::string const Stringf( const char* format, ... );
std::string const Stringf( const int maxLength, const char* format, ... );
std::string const GetAsLowerCase( std::string const & mixedCaseString );
std::vector<std::string> SplitString( std::string const & rawString, char delimiter );
std::vector<size_t> FindIndicies( std::string const & rawString, char delimiter );
bool AllUnique( std::string const & rawString );
void Reverse( char * str );
char * CreateNewCString( std::string const & string );
void SortString( std::string & str );
bool IsPermutation( std::string const & str1, std::string const & str2 );
std::string CompressString( std::string const & str );
int ParseInt( std::string const & string );