#include "Engine/Math/Matrix4fStack.hpp"


//-------------------------------------------------------------------------------------------------
Matrix4fStack::Matrix4fStack( )
{
	m_matricies.push_back( Matrix4f::IDENTITY );
}


//-------------------------------------------------------------------------------------------------
bool Matrix4fStack::IsEmpty() const
{
	return ( m_matricies.size( ) == 1 );
}


//-------------------------------------------------------------------------------------------------
Matrix4f const & Matrix4fStack::GetTop() const
{
	return m_matricies.back( );
}


//-------------------------------------------------------------------------------------------------
void Matrix4fStack::Push( Matrix4f const & mat4 )
{
	Matrix4f top = GetTop( );
	Matrix4f newTop = mat4 * top;
	m_matricies.push_back( newTop );
}


//-------------------------------------------------------------------------------------------------
void Matrix4fStack::Pop( )
{
	if ( !IsEmpty( ) )
	{
		m_matricies.pop_back( );
	}
}