#include "Engine/DebugSystem/Command.hpp"

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
Command::Command( std::string const & command )
{
	unsigned int firstSpace = command.find( ' ' );
	if ( firstSpace == std::string::npos )
	{
		firstSpace = command.length( );
		m_argsString = "";
	}
	else
	{
		m_argsString = command.substr( firstSpace + 1 );
	}
	m_name = command.substr( 0, firstSpace );
	std::string parseList = m_argsString;
	std::string nextArg = GetNextArg( &parseList );
	while ( nextArg != "" )
	{
		m_args.push_back( nextArg );
		nextArg = GetNextArg( &parseList );
	}
}


//-------------------------------------------------------------------------------------------------
std::string const Command::GetName( ) const
{
	return m_name;
}


//-------------------------------------------------------------------------------------------------
bool Command::HasArg( unsigned int argIndex ) const
{
	return argIndex < m_args.size( );
}


//-------------------------------------------------------------------------------------------------
// Override GetArg function for const char *
std::string Command::GetArg( unsigned int argIndex, char const * defaultValue ) const
{
	if ( !HasArg( argIndex ) )
	{
		return std::string( defaultValue );
	}
	std::string result;
	SetTypeFromString<std::string>( result, m_args[argIndex] );
	return result;
}


//-------------------------------------------------------------------------------------------------
std::string Command::GetRemainingString( unsigned int argIndex ) const
{
	std::string result = GetArg( argIndex, "" );
	++argIndex;
	while( HasArg( argIndex ) )
	{
		result.append( " " );
		result.append( GetArg( argIndex, "" ) );
		++argIndex;
	}
	return result;
}


//-------------------------------------------------------------------------------------------------
std::string const Command::GetNextArg( std::string *parseArgList )
{
	if ( *parseArgList == "" )
		return "";

	unsigned int firstSpace = parseArgList->find( ' ' );
	std::string nextArg;
	if ( firstSpace == std::string::npos )
	{
		nextArg = *parseArgList;
		*parseArgList = "";
	}
	else
	{
		nextArg = parseArgList->substr( 0, firstSpace );
		*parseArgList = parseArgList->substr( firstSpace + 1 );
	}

	//Skip single spaces
	if ( nextArg == "" )
		return GetNextArg( parseArgList );
	return nextArg;
}

