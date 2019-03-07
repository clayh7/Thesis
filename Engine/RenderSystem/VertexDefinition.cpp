#include "Engine/RenderSystem/VertexDefinition.hpp"


//-------------------------------------------------------------------------------------------------
VertexDefinition::VertexDefinition( char const * name, unsigned int bindPoint, int count, VertexDataType const & type, bool normalized, int stride, int offset )
	: m_name( name )
	, m_bindPoint( bindPoint )
	, m_count( count )
	, m_type( type )
	, m_normalized( normalized )
	, m_stride( stride )
	, m_offset( offset )
{
}


//-------------------------------------------------------------------------------------------------
VertexDefinition::~VertexDefinition( )
{
}