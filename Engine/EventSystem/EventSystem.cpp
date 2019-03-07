#include "Engine/EventSystem/EventSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
std::map< size_t, std::vector<SubscriberBase*> > * EventSystem::s_registeredSubscribers = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC void EventSystem::Startup( )
{
	if( s_registeredSubscribers == nullptr )
	{
		s_registeredSubscribers = new std::map< size_t, std::vector<SubscriberBase*> >( );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void EventSystem::Shutdown( )
{
	for( auto & subscriptions : *s_registeredSubscribers )
	{
		for( auto deleteMe : subscriptions.second )
		{
			delete deleteMe;
			deleteMe = nullptr;
		}
	}
	s_registeredSubscribers->clear( );

	delete s_registeredSubscribers;
	s_registeredSubscribers = nullptr;
}


//-------------------------------------------------------------------------------------------------
STATIC void EventSystem::RegisterEventAndCommand( std::string const & eventName, std::string const & usage, EventCallback * callback )
{
	g_ConsoleSystem->RegisterCommandEvent( eventName, callback, usage );
	RegisterEvent( eventName, callback );
}


//-------------------------------------------------------------------------------------------------
STATIC void EventSystem::RegisterEvent( std::string const & eventName, EventCallback * callback )
{
	//Find the list of subscriptions under this name
	size_t eventNameHash = std::hash<std::string>{ }( eventName );
	auto foundEventSubscription = s_registeredSubscribers->find( eventNameHash );

	//Create subscriber
	SubscriberStaticFunction * subscriber = new SubscriberStaticFunction( );
	subscriber->m_function = callback;

	//If subscription exists, add to it
	if( foundEventSubscription != s_registeredSubscribers->end( ) )
	{
		std::vector<SubscriberBase*> & eventSubscription = foundEventSubscription->second;
		eventSubscription.push_back( subscriber );
	}

	//If subscription does not exist yet, create one
	else
	{
		std::vector<SubscriberBase*> eventSubscription;
		eventSubscription.push_back( subscriber );
		s_registeredSubscribers->insert( std::pair<size_t, std::vector<SubscriberBase*>>( eventNameHash, eventSubscription ) );
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void EventSystem::TriggerEvent( std::string const & eventName )
{
	NamedProperties empty = NamedProperties();
	TriggerEvent( eventName, empty );
}


//-------------------------------------------------------------------------------------------------
STATIC void EventSystem::TriggerEvent( std::string const & eventName, NamedProperties & eventData )
{
	size_t eventNameHash = std::hash<std::string>{ }( eventName );
	auto foundEventSubscription = s_registeredSubscribers->find( eventNameHash );

	//Event exists
	if( foundEventSubscription != s_registeredSubscribers->end() )
	{
		std::vector<SubscriberBase*> & eventSubscription = foundEventSubscription->second;
		for( SubscriberBase const * subscriber : eventSubscription )
		{
			subscriber->Execute( eventData );
		}
	}

	//Event does not exist, so do nothing
	else
	{
		return;
	}
}


//-------------------------------------------------------------------------------------------------
void EventSystem::TriggerEventForFilesFound( std::string const & eventName, std::string const & baseFolder, std::string const & filePattern )
{
	std::vector < std::string > filesFound = EnumerateFilesInFolder( baseFolder, filePattern );
	for( std::string & relativePath : filesFound )
	{
		NamedProperties fileFoundEvent;
		size_t const MAX_PATH = 260;
		char absolutePath[MAX_PATH];
		_fullpath( absolutePath, relativePath.c_str(), MAX_PATH );

		std::vector<std::string> seperatedPath = SplitString( relativePath, '/' );
		std::string fileName = seperatedPath.back( );
		std::vector<size_t> periodPosition = FindIndicies( fileName, '.' );
		std::vector<std::string> seperatedAbsolutePath = SplitString( absolutePath, '\\' );
		std::string builtAbsolutePath;
		for( std::string & absolutePart : seperatedAbsolutePath )
		{
			builtAbsolutePath += absolutePart;
			builtAbsolutePath += '/';
		}
		builtAbsolutePath.pop_back( );

		fileFoundEvent.Set( "FileName", fileName );
		fileFoundEvent.Set( "FileExtension", fileName.substr( periodPosition.back( ) ) );
		fileFoundEvent.Set( "FileNameWithoutExtension", fileName.substr( 0, periodPosition.back( ) ) );
		fileFoundEvent.Set( "FileRelativePath", relativePath );
		fileFoundEvent.Set( "FileAbsolutePath", builtAbsolutePath );
		
		TriggerEvent( eventName, fileFoundEvent );
	}
}