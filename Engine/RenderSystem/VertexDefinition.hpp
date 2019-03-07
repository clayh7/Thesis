#pragma once

#include <string>
#include <vector>


//-------------------------------------------------------------------------------------------------
enum VertexDataType
{
	VertexDataType_UBYTE,
	VertexDataType_FLOAT,
	VertexDataType_INT,
	VertexDataType_UINT,
};


//-------------------------------------------------------------------------------------------------
class VertexDefinition
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	char const *	m_name;			// Name of this location, nullptr indicates end of the array
	//unsigned int	m_id;			// id from name (GLuint)
	unsigned int	m_bindPoint;	// bind point for to go for (no duplicates)
	int				m_count;		// Number of the following type
	VertexDataType	m_type;			// Type: FLOAT3, UBYTE4, FLOAT2
	bool			m_normalized;	// float3 -> normalizes vector | uchar4 -> changes to float 0-1
	int				m_stride;		// Stride between index locations (sizeof(Vertex_PCT))
	int				m_offset;		// Offset into the vertex where the member variable is located

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	VertexDefinition( char const * name, unsigned int bindPoint, int count, VertexDataType const & type, bool normalized, int stride, int offset );
	~VertexDefinition( );
};