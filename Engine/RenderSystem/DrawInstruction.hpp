#pragma once


#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class DrawInstruction
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	unsigned int m_primitiveType;

public:
	bool m_useIndexBuffer;
	unsigned int m_startIndex;
	unsigned int m_count;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	DrawInstruction( bool useIndexBuffer = false, ePrimitiveType const & primitiveType = ePrimitiveType_TRIANGLES, unsigned int start = 0, unsigned int count = 0 );
	void SetPrimitiveType( ePrimitiveType const & primitiveType );
	unsigned int GetPrimitiveType( ) const;
};