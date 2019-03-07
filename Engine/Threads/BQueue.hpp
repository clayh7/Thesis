#pragma once
#include <queue>
#include "Engine/Memory/MemoryAnalytics.hpp"
#include "Engine/Threads/CriticalSection.hpp"


//-------------------------------------------------------------------------------------------------
// Bread Engine Thread-safe Queue
template<typename Type>
class BQueue : protected std::queue<Type>
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	CriticalSection m_criticalSection;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	//-------------------------------------------------------------------------------------------------
	BQueue( )
	{

	}

	//---------------------------------------------------------------------------------------------
	void PushBack( Type const & value )
	{
		m_criticalSection.Lock( );
		push( value );
		m_criticalSection.Unlock( );
	}

	//---------------------------------------------------------------------------------------------
	bool PopFront( Type * out_value )
	{
		bool result = false;
		m_criticalSection.Lock( );
		if( !empty() )
		{
			*out_value = front();
			pop( );
			result = true;
		}
		m_criticalSection.Unlock( );
		return result;
	}
};