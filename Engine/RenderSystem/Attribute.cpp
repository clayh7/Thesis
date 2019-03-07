#include "Engine/RenderSystem/Attribute.hpp"


//-------------------------------------------------------------------------------------------------
Attribute::Attribute( int bindPoint, int length, int size, unsigned int type, std::string const & name, void * const data )
	: m_bindPoint( bindPoint )
	, m_length( length )
	, m_size( size )
	, m_type( type )
	, m_name( name )
	, m_data( data )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Attribute::~Attribute( )
{
	delete m_data;
}
