#pragma once

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"

//-------------------------------------------------------------------------------------------------
template<typename Item>
class Array
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static size_t const DEFAULT_SIZE = 8U;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	Item * m_data;
	size_t m_count;
	size_t m_maxSize;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Array( size_t size = DEFAULT_SIZE )
		: m_data( nullptr )
		, m_count( 0U )
		, m_maxSize( size )
	{
		if( size > 0U )
		{
			m_data = new Item[size];
		}
	}

	~Array( )
	{
		delete m_data;
	}

	Item & operator[]( size_t index )
	{
		return m_data[index];
	}

	Item const & operator[]( size_t index ) const
	{
		return m_data[index];
	}

	void Push( Item const & add )
	{
		if( m_count == m_maxSize )
		{
			Resize( m_maxSize * 2 + 1 );
		}
		m_data[m_count] = add;
		++m_count;
	}

	Item Pop( )
	{
		--m_count;
		return m_data[m_count];
	}

	Item Remove( size_t index )
	{
		Item removed = m_data[index];
		for( size_t shift = index; shift < m_count; ++shift )
		{
			m_data[shift] = m_data[shift + 1];
		}
		--m_count;
		return removed;
	}

	size_t Size( ) const
	{
		return m_count;
	}

	void DeleteAndClear( )
	{
		for( size_t index = 0; index < m_count; ++index )
		{
			delete m_data[index];
			m_data[index] = nullptr;
		}
		Clear( );
	}

	void Clear( )
	{
		m_count = 0;
	}

	void Resize( size_t newSize )
	{
		Item * m_temp = new Item[newSize];
		memcpy( m_temp, m_data, sizeof( Item ) * m_count );
		delete m_data;
		m_data = m_temp;
		m_maxSize = newSize;
	}
};