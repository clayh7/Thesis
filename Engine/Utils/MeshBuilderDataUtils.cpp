#include "Engine/Utils/MeshBuilderDataUtils.hpp"

#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
PlaneData::PlaneData( )
{
	//Dirty Values
}


//-------------------------------------------------------------------------------------------------
Vector3f PlaneFunc( void const *userArgs, float x, float y )
{
	PlaneData const *plane = (PlaneData const*) userArgs;
	Vector3f position = plane->initialPosition
		+ x * plane->rightVector
		+ y * plane->upVector;

	return position;
}


//-------------------------------------------------------------------------------------------------
Vector3f WavesFunc( void const *userArgs, float x, float y )
{
	//Extract Data
	PlaneData const *plane = (PlaneData const*) userArgs;

	float z = 0;
	if( plane->effectIndex == 0 )
	{
		z = sin( x * y );
	}
	else if( plane->effectIndex == 1 )
	{
		z = x * y * y * y - y * x * x * x;
	}
	else if( plane->effectIndex == 2 )
	{
		z = cos( abs( x ) + abs( y ) );
	}
	else if( plane->effectIndex == 3 )
	{
		z = cos( abs( x ) + abs( y ) )*( abs( x ) + abs( y ) );
	}
	else if( plane->effectIndex == 4 )
	{
		z = sin( sqrt( x*x + y*y ) );
	}

	Vector3f normal = CrossProduct( plane->rightVector, plane->upVector );
	Vector3f position = plane->initialPosition
		+ x * plane->rightVector
		+ y * plane->upVector
		+ z * normal;

	return position;
}