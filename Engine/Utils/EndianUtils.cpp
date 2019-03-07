#include "Engine/Utils/EndianUtils.hpp"


//-------------------------------------------------------------------------------------------------
eEndianMode GetLocalEndianess( )
{
	//Alias 2 variables to make them on the same memory
	union
	{
		byte_t bdata[4];
		uint32_t uidata;
	} data;

	data.uidata = 0x04030201;
	//Index:3	  2		1	  0
	//Data: [0x04][0x03][0x02][0x01]
	if( data.bdata[0] == 0x01 )
	{
		return eEndianMode_LITTLE_ENDIAN;
	}
	//Index:3	  2		1	  0
	//Data: [0x01][0x02][0x03][0x04]
	else
	{
		return eEndianMode_BIG_ENDIAN;
	}
}