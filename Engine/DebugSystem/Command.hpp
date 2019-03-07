#pragma once

#include <string>
#include <vector>
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
class Command
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	std::string m_name;
	std::string m_argsString;
	std::vector<std::string> m_args;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Command( std::string const & command );

	std::string const GetName( ) const;
	bool HasArg( unsigned int argIndex ) const;
	std::string GetArg( unsigned int argIndex, char const * defaultValue ) const;
	std::string GetRemainingString( unsigned int argIndex ) const;

private:
	std::string const GetNextArg( std::string * parseArgList );

//-------------------------------------------------------------------------------------------------
// Function Templates
//-------------------------------------------------------------------------------------------------
public:
	template< typename ArgType >
	ArgType GetArg( unsigned int argIndex, ArgType const & defaultValue ) const
	{
		if( !HasArg( argIndex ) )
		{
			return defaultValue;
		}
		ArgType result;
		SetTypeFromString<ArgType>( result, m_args[argIndex] );
		return result;
	}
};