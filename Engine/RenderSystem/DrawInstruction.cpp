#include "Engine/RenderSystem/DrawInstruction.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
DrawInstruction::DrawInstruction( bool useIndexBuffer /*= false*/, ePrimitiveType const & primitiveType /*= PrimitiveType_TRIANGLES*/, unsigned int start /*= 0*/, unsigned int count /*= 0 */ )
	: m_primitiveType( 0x0004 )
	, m_startIndex( start )
	, m_count( count )
	, m_useIndexBuffer( useIndexBuffer )
{
	SetPrimitiveType( primitiveType );
}


//-------------------------------------------------------------------------------------------------
void DrawInstruction::SetPrimitiveType( ePrimitiveType const & primitiveType )
{
	switch ( primitiveType )
	{
	case ePrimitiveType_TRIANGLES:
		m_primitiveType = 0x0004; //GL_TRIANGLES
		break;
	case ePrimitiveType_LINES:
		m_primitiveType = 0x0001; //GL_LINES
		break;
	default:
		ERROR_AND_DIE( "Primitive not handled" );
		break;
	}
}


//-------------------------------------------------------------------------------------------------
unsigned int DrawInstruction::GetPrimitiveType( ) const
{
	return m_primitiveType;
}
