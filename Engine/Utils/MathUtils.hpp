#pragma once

#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
class Vector2i;
class Vector2f;
class Vector3f;
class Vector4f;
class AABB2f;
class Matrix4f;


//-------------------------------------------------------------------------------------------------
extern const float PI;
extern const float fSQRT_3_OVER_3;
extern const float s_sinValuesForByteAngles[256];
extern const uint8_t MAX_UINT8;
extern const uint16_t MAX_UINT16;
extern const uint32_t MAX_UINT32;


//-------------------------------------------------------------------------------------------------
float DegToRad( float degrees );
float RadToDeg( float radians );
float SinDegrees( float degrees );
float CosDegrees( float degrees );
float Atan2Degrees( float y, float x );
float Atan2Degrees( Vector2f const & direction );
float SinDegreesFast( float degrees );
float CosDegreesFast( float degrees );
float WrapDegrees0to360( float degrees );
float ShortestSignedAngularDistance( float startDegrees, float endDegrees );

float RandomFloatZeroToOne( );
float RandomFloat( float randomCap );
float RandomFloat( float randomMin, float randomMax );
int RandomIntZeroToMax( );
int RandomInt( int randomCap );
int RandomInt( int randomMin, int randomMax );
int RandomInt( Vector2i const & randomRange );
int RandomIntInclusive( Vector2i const & randomRange );
Vector2f RandomUnitVectorCircle( );
Vector3f RandomUnitVectorSphere( );
Vector2f RandomPointAABB2( AABB2f const & bounds );
Vector2f UnitVectorFromDegrees( float degrees );

float DistanceBetweenPoints( Vector2f const & position1, Vector2f const & position2 );
float DistanceBetweenPoints( Vector3f const & position1, Vector3f const & position2 );
float DistanceBetweenPointsSquared( Vector2f const & position1, Vector2f const & position2 );
float DistanceBetweenPointsSquared( Vector3f const & position1, Vector3f const & position2 );

bool OverlapDiscs( Vector2f const & center1, float radius1, Vector2f const & center2, float radius2 );
bool OverlapAABB2s( AABB2f const & box1, AABB2f const box2 );
bool OverlapDiscAndAABB2( Vector2f const & center, float radius, AABB2f const & box );

float Lerp( float start, float end, float fractionToEnd );
Vector2f Lerp( Vector2f const & start, Vector2f const & end, float fractionToEnd );
Vector3f Lerp( Vector3f const & start, Vector3f const & end, float fractionToEnd );
float LerpDegrees( float start, float end, float fractionToEnd );
Matrix4f LerpMatrix( Matrix4f const & start, Matrix4f const & end, float fractionToEnd );
float RangeMapNormalize( float inputStart, float inputEnd, float inputValue );
float RangeMap( float inputStart, float inputEnd, float inputValue, float outputStart, float outputEnd );
float Clamp( float numToClamp, float minNum, float maxNum );
int Clamp( int numToClamp, int minNum, int maxNum );
Vector2f Clamp( Vector2f const & vecToClamp, Vector2f const & minVec, Vector2f const & maxVec );
float Min( float num1, float num2 );
int Min( int num1, int num2 );
size_t Min( size_t num1, size_t num2 );
uint16_t Min( uint16_t num1, uint16_t num2 );
float Max( float num1, float num2 );
int Max( int num1, int num2 );
size_t Max( size_t num1, size_t num2 );
int RoundToNearestInt( float numToRound );
float FastFloor( float f );
int FastFloorToInt( float f );
bool GreaterThanCycle( uint16_t num1, uint16_t num2 );

uint8_t CompressFloat32ToUint8( float actual, float min, float max );
float DecompressUint8ToFloat32( uint8_t compressed, float min, float max );
uint16_t CompressFloat32ToUint16( float actual, float min, float max );
float DecompressUint16ToFloat32( uint16_t compressed, float min, float max );
uint32_t CompressVector2fToUint32( Vector2f const & actual, Vector2f const & min, Vector2f const & max );
Vector2f DecompressUint32ToVector2f( uint32_t compressed, Vector2f const & min, Vector2f const & max );

float SmoothStep( float inputZeroToOne );
float SmootherStep( float inputZeroToOne );
float DotProduct( Vector2f const & vector1, Vector2f const & vector2 );
float DotProduct( Vector3f const & vector1, Vector3f const & vector2 );
float DotProduct( Vector4f const & vector1, Vector4f const & vector2 );
const Vector3f CrossProduct( Vector3f const & vector1, Vector3f const & vector2 );
float GetPseudoRandomNoise1D( int position );
float GetPseudoRandomNoise2D( int positionX, int positionY );
float GetPseudoRandomNoise3D( int positionX, int positionY, int positionZ );
Vector2f GetPseudoRandomNoiseDirection2D( int positionX, int positionY );

bool IsBitfieldSet( uint16_t bitFlags, size_t bitMask );
bool IsBitfieldSet( uint8_t bitFlags, size_t bitMask );
bool IsBitSet( uint16_t bitFlags, size_t bit );
bool IsBitSet( uint8_t bitFlags, size_t bit );
void SetBit( uint16_t * out_bitField, size_t bitToSet );
void SetBit( uint8_t * out_bitField, size_t bitToSet );
size_t HashMemory( void const * memory, size_t const memorySize );

template<typename Item>
void MergeSort( Item * itemArray, size_t length )
{
	Item * helper = new Item[length];
	MergeSort( itemArray, helper, 0, length - 1 );
	delete helper;
}

template<typename Item>
void MergeSort( Item * itemArray, Item * helper, int low, int high )
{
	if( low < high )
	{
		int middle = ( low + high ) / 2;
		MergeSort( itemArray, helper, low, middle ); //Left half
		MergeSort( itemArray, helper, middle + 1, high ); //Right half
		Merge( itemArray, helper, low, middle, high );
	}
}

template<typename Item>
void Merge( Item * itemArray, Item * helper, int low, int middle, int high )
{
	//Copy array to helper
	for( int i = low; i <= high; ++i )
	{
		helper[i] = itemArray[i];
	}

	int helperLeft = low;
	int helperRight = middle + 1;
	int current = low;

	//Compare and copy into original array
	while( helperLeft <= middle && helperRight <= high )
	{
		if( helper[helperLeft] <= helper[helperRight] )
		{
			itemArray[current] = helper[helperLeft];
			++helperLeft;
		}
		else
		{
			itemArray[current] = helper[helperRight];
			++helperRight;
		}
		++current;
	}

	//Copy whats left of the left side
	while( helperLeft <= middle )
	{
		itemArray[current] = helper[helperLeft];
		++current;
		++helperLeft;
	}

	//The right side is already in the array
}