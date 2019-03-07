#include "Engine/Core/NamedProperties.hpp"


//-------------------------------------------------------------------------------------------------
NamedProperties::NamedProperties( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
NamedProperties::~NamedProperties( )
{
	for( std::pair<size_t, NamedPropertyBase*> property : m_properties )
	{
		delete property.second;
		property.second = nullptr;
	}
	m_properties.clear( );
}


//-------------------------------------------------------------------------------------------------
//Override specifically to handle (char *) as (std::string)
ePropertySetResult NamedProperties::Set( std::string const & propertyName, char * propertyValue )
{
	return Set( propertyName, static_cast<char const *>( propertyValue ) );
}


//-------------------------------------------------------------------------------------------------
//Override specifically to handle (char const *) as (std::string)
ePropertySetResult NamedProperties::Set( std::string const & propertyName, char const * propertyValue )
{
	size_t propertyNameHash = std::hash<std::string>{ } ( propertyName );
	auto found = m_properties.find( propertyNameHash );

	//Could not find property
	if( found == m_properties.end( ) )
	{
		NamedProperty<std::string> * addProperty = new NamedProperty<std::string>( propertyValue );
		m_properties.insert( std::pair<size_t, NamedPropertyBase*>( propertyNameHash, addProperty ) );
		return ePropertySetResult_SUCCESS_ADDED;
	}

	//Found property
	else
	{
		//Try to cast to the propertyValue type, if it's not the propertyValue type this'll return null
		NamedPropertyBase * property = found->second;
		NamedProperty<std::string> * typedProperty = dynamic_cast<NamedProperty<std::string>*>( property );

		//Property is correct type
		if( typedProperty )
		{
			typedProperty->m_data = propertyValue;
			return ePropertySetResult_SUCCESS_UPDATE;
		}

		//Property is not correct type
		else
		{
			delete found->second;
			found->second = new NamedProperty<std::string>( propertyValue );
			return ePropertySetResult_SUCCESS_CHANGED_TYPE;
		}
	}
}