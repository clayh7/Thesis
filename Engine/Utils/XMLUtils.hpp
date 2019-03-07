#pragma once

#include <string>
#include "ThirdParty/Parsers/XMLParser.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
std::string			GetXMLErrorDescForErrorCode( XMLError xmlErrorCode );
bool				GetXMLNodeByNameSearchingFromPosition( const XMLNode& parentNode, std::string const & childName, int& position_inout, XMLNode& childNode_out );
std::string			GetXMLAttributeAsString( const XMLNode& node, std::string const & attributeName, bool& wasAttributePresent_out );
void				DestroyXMLDocument( XMLNode& xmlDocumentToDestroy );


//-------------------------------------------------------------------------------------------------
// Maybe add this to a UtilsCommon
//-------------------------------------------------------------------------------------------------
template< typename DestinationType >
void SetTypeFromString( DestinationType & out_destination, std::string const &asString )
{
	const DestinationType constructedFromString( asString ); //Add conversion function below
	out_destination = constructedFromString;
}


//-------------------------------------------------------------------------------------------------
template< typename DestinationType >
void SetTypeFromString( DestinationType* out_destination, std::string const &asString )
{
	UNREFERENCED( asString );
	out_destination = nullptr;
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( size_t & out_destination, std::string const &asString )
{
	if( asString.size( ) == 0 )
	{
		out_destination = 0U;
	}
	else
	{
		out_destination = (size_t) std::stoul( asString.c_str( ) );
	}
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( int & out_destination, std::string const &asString )
{
	out_destination = ( int ) atoi( asString.c_str( ) );
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( float & out_destination, std::string const &asString )
{
	out_destination = ( float ) atof( asString.c_str( ) );
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( double & out_destination, std::string const &asString )
{
	out_destination = atof( asString.c_str( ) );
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( char & out_destination, std::string const &asString )
{
	out_destination = ( char ) asString[0];
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( unsigned char & out_destination, std::string const &asString )
{
	out_destination = (unsigned char) std::stoul( asString.c_str( ) );
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( bool & out_destination, std::string const &asString )
{
	if( asString == "true" )
	{
		out_destination = true;
	}
	else if( asString == "false" )
	{
		out_destination = false;
	}
	else if( asString == "" )
	{
		out_destination = false;
	}
	else
	{
		ERROR_AND_DIE( "Invalid conversion from string to bool." );
	}
}


//-------------------------------------------------------------------------------------------------
template< > inline void SetTypeFromString( uint16_t & out_destination, std::string const &asString )
{
	if( asString.size( ) == 0 )
	{
		out_destination = 0U;
	}
	else
	{
		out_destination = (uint16_t) std::stoul( asString.c_str( ) );
	}
}


//-------------------------------------------------------------------------------------------------
// Use this example for specific override
//template< > inline void SetTypeFromString( eDirection &typedObject, std::string const &asString )
//{
//	typedObject = GetDirectionEnumForName( asString );
//}


//-------------------------------------------------------------------------------------------------
template< typename ValueType >
ValueType GetXMLAttributeOfType( const XMLNode& node, std::string const & propertyName, bool& wasPropertyPresent_out )
{
	ValueType outValue;
	std::string	valueAsString = GetXMLAttributeAsString( node, propertyName, wasPropertyPresent_out );
	SetTypeFromString( outValue, valueAsString );

	return outValue;
}


//-------------------------------------------------------------------------------------------------
template< typename ValueType >
ValueType ReadXMLAttribute( const XMLNode& node, std::string const & propertyName, const ValueType& defaultValue )
{
	bool wasPropertyPresent = false;

	ValueType outValue = GetXMLAttributeOfType< ValueType >( node, propertyName, wasPropertyPresent );
	if ( !wasPropertyPresent )
		outValue = defaultValue;

	return outValue;
}


//-------------------------------------------------------------------------------------------------
template< typename ValueType >
void WriteXMLAttribute( XMLNode& node, std::string const & propertyName, ValueType& value, const ValueType& defaultValue )
{
	SetStringProperty( node, propertyName, GetTypedObjectAsString( value ), GetTypedObjectAsString( defaultValue ) );
}


//-------------------------------------------------------------------------------------------------
std::string ReadXMLAttribute( const XMLNode& node, std::string const & propertyName, const char * defaultValue );