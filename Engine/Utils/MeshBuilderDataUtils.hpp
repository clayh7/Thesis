#pragma once

#include "Engine/Math/Vector3f.hpp"


//-------------------------------------------------------------------------------------------------
class PlaneData
{
public:
	Vector3f initialPosition;
	Vector3f rightVector;
	Vector3f upVector;
	int effectIndex;

public:
	PlaneData( );
};


//-------------------------------------------------------------------------------------------------
typedef Vector3f( PatchFunc )( void const*, float x, float y );
Vector3f PlaneFunc( void const * userArgs, float x, float y );
Vector3f WavesFunc( void const * userArgs, float x, float y );