#include "Engine/Net/Session/NetMessageDefinition.hpp"

#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
void NetMessageDefinition::CalculateHeaderSize( )
{
	//message type 1 byte
	headerSize = 1;

	//sender index 1 byte
	if( !IsConnectionless( ) )
	{
		headerSize += 1;
	}

	//reliable id 2 bytes
	if( IsReliable( ) )
	{
		headerSize += 2;
	}

	if( IsReliable( ) && IsSequence( ) )
	{
		headerSize += 2;
	}
}


//-------------------------------------------------------------------------------------------------
bool NetMessageDefinition::IsConnectionless( ) const
{
	return IsBitfieldSet( controlFlags, CONNECTIONLESS_CONTROL_FLAG );
}


//-------------------------------------------------------------------------------------------------
bool NetMessageDefinition::IsReliable( ) const
{
	return IsBitfieldSet( optionFlags, RELIABLE_OPTION_FLAG );
}


//-------------------------------------------------------------------------------------------------
bool NetMessageDefinition::IsSequence( ) const
{
	return IsBitfieldSet( optionFlags, SEQUENCE_OPTION_FLAG );
}