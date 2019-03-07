#pragma once

#include <string>
#include <map>
#include <vector>
#include "Engine/DebugSystem/Console.hpp"


//-------------------------------------------------------------------------------------------------
class NamedProperties;
typedef void ( EventCallback )( NamedProperties & );


//-----------------------------------------------------------------------------------------------
class SubscriberBase
{
public:
	virtual void Execute( NamedProperties & ) const = 0;
	virtual void * GetObject( ) const = 0;
};


//-------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_FunctionType>
class SubscriberObjectFunction : public SubscriberBase
{
public:
	T_ObjectType * m_object;
	T_FunctionType m_function;

public:
	virtual void Execute( NamedProperties & params ) const override
	{
		( m_object->*m_function )( params );
	}

private:
	virtual void * GetObject( ) const
	{
		return (void*) m_object;
	}
};


//-------------------------------------------------------------------------------------------------
class SubscriberStaticFunction : public SubscriberBase
{
public:
	EventCallback * m_function;

public:
	virtual void Execute( NamedProperties & params ) const override
	{
		( m_function )( params );
	}

private:
	virtual void * GetObject( ) const
	{
		return nullptr;
	}
};


//-----------------------------------------------------------------------------------------------
class EventSystem
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static std::map< size_t, std::vector<SubscriberBase*> > * s_registeredSubscribers;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void Startup( );
	static void Shutdown( );
	static void RegisterEventAndCommand( std::string const & eventName, std::string const & usage, EventCallback * callback );
	static void RegisterEvent( std::string const & eventName, EventCallback * callback );
	static void TriggerEvent( std::string const & eventName );
	static void TriggerEvent( std::string const & eventName, NamedProperties & eventData );
	static void TriggerEventForFilesFound( std::string const & eventName, std::string const & baseFolder, std::string const & filePattern );

//-------------------------------------------------------------------------------------------------
// Static Function Templates
//-------------------------------------------------------------------------------------------------
public:
	template <typename T_ObjectType, typename T_FunctionType>
	static void RegisterEventAndCommand( std::string const & eventName, std::string const & usage, T_ObjectType * object, T_FunctionType function )
	{
		g_ConsoleSystem->RegisterCommandEvent( eventName, object, function, usage );
		RegisterEvent( eventName, object, function );
	}
	
	template <typename T_ObjectType, typename T_FunctionType>
	static void RegisterEvent( std::string const & eventName, T_ObjectType * object, T_FunctionType function )
	{
		if( s_registeredSubscribers == nullptr )
		{
			EventSystem::Startup( );
		}

		//Find the list of subscriptions under this name
		size_t eventNameHash = std::hash<std::string>{ }( eventName );
		auto foundEventSubscription = s_registeredSubscribers->find( eventNameHash );

		//Create subscriber
		SubscriberObjectFunction<T_ObjectType, T_FunctionType> * subscriber = new SubscriberObjectFunction<T_ObjectType, T_FunctionType>( );
		subscriber->m_object = object;
		subscriber->m_function = function;

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

	//Remove the subscriber from all of their Registered Events
	template <typename T_ObjectType>
	static void EventSystem::Unregister( T_ObjectType * subscriber )
	{
		//std::pair<size_t,std::vector<SubscriberBase*>> const 
		for( auto & eventSubscriptionPair : *s_registeredSubscribers )
		{
			std::vector<SubscriberBase*> & eventSubscription = eventSubscriptionPair.second;
			for( auto subscriberIter = eventSubscription.begin( ); subscriberIter != eventSubscription.end( ); /*Nothing*/ )
			{
				SubscriberBase * subscriberBase = *subscriberIter;
				T_ObjectType * object = static_cast<T_ObjectType*>( subscriberBase->GetObject( ) );

				//Make sure subscriber is the correct type
				if( object )
				{
					//Make sure subscriber matches our criteria
					if( object == subscriber )
					{
						//Remove the subscriber
						delete subscriberBase;
						subscriberIter = eventSubscription.erase( subscriberIter );
					}
					else
					{
						++subscriberIter;
					}
				}
				else
				{
					++subscriberIter;
				}
			}
		}
	}
};