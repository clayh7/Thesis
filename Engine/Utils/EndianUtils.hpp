#pragma once

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
//LITTLE_ENDIAN
//[0x04][0x03][0x02][0x01]
//
//BIG_ENDIAN
//[0x01][0x02][0x03][0x04]


//-------------------------------------------------------------------------------------------------
enum eEndianMode
{
	eEndianMode_LITTLE_ENDIAN, //LEAST SIGNIFICANT
	eEndianMode_BIG_ENDIAN, //MOST SIGNIFICANT
};


//-------------------------------------------------------------------------------------------------
eEndianMode GetLocalEndianess( );