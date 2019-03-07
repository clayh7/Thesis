#include "Engine/Utils/MathUtils.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/Math/Vector2i.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4f.hpp"


//-------------------------------------------------------------------------------------------------
const float PI = 3.1415926535897932384626433832795f;


//-------------------------------------------------------------------------------------------------
const float fSQRT_3_OVER_3 = 0.5773502691896257645091f;


//-----------------------------------------------------------------------------------------------
// Lookup table for sin() values for 256 evenly distributed angles around the unit circle,
//	where Byte 0 = 0 degrees, Byte 64 = 90 degrees, Byte 128 = 180 degrees, etc.
// Credit: Squirrel Eiserloh
const float s_sinValuesForByteAngles[256] = {
	0.000000000f, 0.024541229f, 0.049067674f, 0.073564564f, 0.098017140f, 0.122410675f, 0.146730474f, 0.170961889f,
	0.195090322f, 0.219101240f, 0.242980180f, 0.266712757f, 0.290284677f, 0.313681740f, 0.336889853f, 0.359895037f,
	0.382683432f, 0.405241314f, 0.427555093f, 0.449611330f, 0.471396737f, 0.492898192f, 0.514102744f, 0.534997620f,
	0.555570233f, 0.575808191f, 0.595699304f, 0.615231591f, 0.634393284f, 0.653172843f, 0.671558955f, 0.689540545f,
	0.707106781f, 0.724247083f, 0.740951125f, 0.757208847f, 0.773010453f, 0.788346428f, 0.803207531f, 0.817584813f,
	0.831469612f, 0.844853565f, 0.857728610f, 0.870086991f, 0.881921264f, 0.893224301f, 0.903989293f, 0.914209756f,
	0.923879533f, 0.932992799f, 0.941544065f, 0.949528181f, 0.956940336f, 0.963776066f, 0.970031253f, 0.975702130f,
	0.980785280f, 0.985277642f, 0.989176510f, 0.992479535f, 0.995184727f, 0.997290457f, 0.998795456f, 0.999698819f,
	1.000000000f, 0.999698819f, 0.998795456f, 0.997290457f, 0.995184727f, 0.992479535f, 0.989176510f, 0.985277642f,
	0.980785280f, 0.975702130f, 0.970031253f, 0.963776066f, 0.956940336f, 0.949528181f, 0.941544065f, 0.932992799f,
	0.923879533f, 0.914209756f, 0.903989293f, 0.893224301f, 0.881921264f, 0.870086991f, 0.857728610f, 0.844853565f,
	0.831469612f, 0.817584813f, 0.803207531f, 0.788346428f, 0.773010453f, 0.757208847f, 0.740951125f, 0.724247083f,
	0.707106781f, 0.689540545f, 0.671558955f, 0.653172843f, 0.634393284f, 0.615231591f, 0.595699304f, 0.575808191f,
	0.555570233f, 0.534997620f, 0.514102744f, 0.492898192f, 0.471396737f, 0.449611330f, 0.427555093f, 0.405241314f,
	0.382683432f, 0.359895037f, 0.336889853f, 0.313681740f, 0.290284677f, 0.266712757f, 0.242980180f, 0.219101240f,
	0.195090322f, 0.170961889f, 0.146730474f, 0.122410675f, 0.098017140f, 0.073564564f, 0.049067674f, 0.024541229f,
	0.000000000f, -0.024541229f, -0.049067674f, -0.073564564f, -0.098017140f, -0.122410675f, -0.146730474f, -0.170961889f,
	-0.195090322f, -0.219101240f, -0.242980180f, -0.266712757f, -0.290284677f, -0.313681740f, -0.336889853f, -0.359895037f,
	-0.382683432f, -0.405241314f, -0.427555093f, -0.449611330f, -0.471396737f, -0.492898192f, -0.514102744f, -0.534997620f,
	-0.555570233f, -0.575808191f, -0.595699304f, -0.615231591f, -0.634393284f, -0.653172843f, -0.671558955f, -0.689540545f,
	-0.707106781f, -0.724247083f, -0.740951125f, -0.757208847f, -0.773010453f, -0.788346428f, -0.803207531f, -0.817584813f,
	-0.831469612f, -0.844853565f, -0.857728610f, -0.870086991f, -0.881921264f, -0.893224301f, -0.903989293f, -0.914209756f,
	-0.923879533f, -0.932992799f, -0.941544065f, -0.949528181f, -0.956940336f, -0.963776066f, -0.970031253f, -0.975702130f,
	-0.980785280f, -0.985277642f, -0.989176510f, -0.992479535f, -0.995184727f, -0.997290457f, -0.998795456f, -0.999698819f,
	-1.000000000f, -0.999698819f, -0.998795456f, -0.997290457f, -0.995184727f, -0.992479535f, -0.989176510f, -0.985277642f,
	-0.980785280f, -0.975702130f, -0.970031253f, -0.963776066f, -0.956940336f, -0.949528181f, -0.941544065f, -0.932992799f,
	-0.923879533f, -0.914209756f, -0.903989293f, -0.893224301f, -0.881921264f, -0.870086991f, -0.857728610f, -0.844853565f,
	-0.831469612f, -0.817584813f, -0.803207531f, -0.788346428f, -0.773010453f, -0.757208847f, -0.740951125f, -0.724247083f,
	-0.707106781f, -0.689540545f, -0.671558955f, -0.653172843f, -0.634393284f, -0.615231591f, -0.595699304f, -0.575808191f,
	-0.555570233f, -0.534997620f, -0.514102744f, -0.492898192f, -0.471396737f, -0.449611330f, -0.427555093f, -0.405241314f,
	-0.382683432f, -0.359895037f, -0.336889853f, -0.313681740f, -0.290284677f, -0.266712757f, -0.242980180f, -0.219101240f,
	-0.195090322f, -0.170961889f, -0.146730474f, -0.122410675f, -0.098017140f, -0.073564564f, -0.049067674f, -0.024541229f
};


//-------------------------------------------------------------------------------------------------
uint8_t const MAX_UINT8 = (uint8_t) ( (uint8_t) 0U - (uint8_t) 1U );
uint16_t const MAX_UINT16 = (uint16_t) ( (uint16_t) 0U - (uint16_t) 1U );
uint32_t const MAX_UINT32 = ( 0U - 1U );

//-------------------------------------------------------------------------------------------------
float DegToRad( float degrees )
{
	return ( degrees / 180.f ) * PI;
}


//-------------------------------------------------------------------------------------------------
float RadToDeg( float radians )
{
	return ( radians / PI ) * 180.f;
}


//-------------------------------------------------------------------------------------------------
float SinDegrees( float degrees )
{
	return sin( DegToRad( degrees ) );
}


//-------------------------------------------------------------------------------------------------
float CosDegrees( float degrees )
{
	return cos( DegToRad( degrees ) );
}


//-------------------------------------------------------------------------------------------------
float Atan2Degrees( float y, float x )
{
	return RadToDeg( atan2( y, x ) );
}


//-------------------------------------------------------------------------------------------------
float Atan2Degrees( const Vector2f& direction )
{
	return RadToDeg( atan2( direction.y, direction.x ) );
}


//-------------------------------------------------------------------------------------------------
float SinDegreesFast( float degrees )
{
	float oneOver360 = 1.f / 360.f;
	degrees = WrapDegrees0to360( degrees );
	unsigned char convertedDegrees = ( unsigned char ) ( ( 256.f ) * degrees * oneOver360 );
	return s_sinValuesForByteAngles[convertedDegrees];
}


//-------------------------------------------------------------------------------------------------
float CosDegreesFast( float degrees )
{
	return SinDegreesFast( 90.f - degrees );
}


//-------------------------------------------------------------------------------------------------
float WrapDegrees0to360( float degrees )
{
	float start = 0.f;
	float end = 360.f;
	float range = end - start;
	float offset = degrees - start;
	return ( offset - ( floor( offset / range ) * range ) ) + start;
}


//-------------------------------------------------------------------------------------------------
float WrapDegrees180to180( float degrees )
{
	float start = -180.f;
	float end = 180.f;
	float range = end - start;
	float offset = degrees - start;
	return ( offset - ( floor( offset / range ) * range ) ) + start;
}


//-------------------------------------------------------------------------------------------------
float ShortestSignedAngularDistance( float startDegrees, float endDegrees )
{
	float degreeDifference = endDegrees - startDegrees;
	return WrapDegrees180to180( degreeDifference );
}


//-------------------------------------------------------------------------------------------------
//Returns float [0 - 1) [including 0, excluding 1)
float RandomFloatZeroToOne( )
{
	return rand( ) / ( RAND_MAX + 1.f );
}


//-------------------------------------------------------------------------------------------------
//Returns float [0 - randomCap) [including 0, excluding randomCap)
float RandomFloat( float randomCap )
{
	return RandomFloatZeroToOne( ) * randomCap;
}


//-------------------------------------------------------------------------------------------------
//Returns int [randomMin - randomMax) [including randomMin, excluding randomMax)
float RandomFloat( float randomMin, float randomMax )
{
	float range = randomMax - randomMin;
	return RandomFloat( range ) + randomMin;
}


//-------------------------------------------------------------------------------------------------
//Returns int [0 - RAND_MAX] [including 0, including RAND_MAX]
int RandomIntZeroToMax( )
{
	return rand( );
}


//-------------------------------------------------------------------------------------------------
//Returns int [0 - randomCap) [including 0, excluding randomCap)
int RandomInt( int randomCap )
{
	if ( randomCap > 0 )
		return rand( ) % randomCap;
	else
		return 0;
}


//-------------------------------------------------------------------------------------------------
//Returns int [randomMin - randomMax) [including randomMin, excluding randomMax)
int RandomInt( int randomMin, int randomMax )
{
	int range = randomMax - randomMin;
	return RandomInt( range ) + randomMin;
}


//-------------------------------------------------------------------------------------------------
//Returns int [randomRange.x - randomRange.y) [including randomRange.x, excluding randomRange.y)
int RandomInt( Vector2i const & randomRange )
{
	return RandomInt( randomRange.x, randomRange.y );
}


//-------------------------------------------------------------------------------------------------
//Returns int [randomRange.x - randomRange.y] [including randomRange.x, excluding randomRange.y]
int RandomIntInclusive( Vector2i const & randomRange )
{
	return RandomInt( randomRange.x, randomRange.y + 1 );
}


//-------------------------------------------------------------------------------------------------
Vector2f RandomUnitVectorCircle( )
{
	float degrees = RandomFloat( 360.f );
	return Vector2f( CosDegrees( degrees ), SinDegrees( degrees ) );
}


//-------------------------------------------------------------------------------------------------
//#TODO: Make faster?
Vector3f RandomUnitVectorSphere( )
{
	float constraint;
	float randX;
	float randY;
	do
	{
		randX = ( RandomFloatZeroToOne( ) * 2 ) - 1.f;
		randY = ( RandomFloatZeroToOne( ) * 2 ) - 1.f;
		constraint = ( randX * randX ) + ( randY * randY );
	}
	while ( constraint >= 1.f );

	float randX2 = randX * randX;
	float randY2 = randY * randY;
	float sqrtXY = sqrt( 1 - randX2 - randY2 );
	float unitX = 2.f * randX * sqrtXY;
	float unitY = 2.f * randY * sqrtXY;
	float unitZ = 1 - 2.f * ( randX2 + randY2 );
	return Vector3f( unitX, unitY, unitZ );
}


//-------------------------------------------------------------------------------------------------
Vector2f RandomPointAABB2( AABB2f const & bounds )
{
	float xCoord = RandomFloat( bounds.mins.x, bounds.maxs.x );
	float yCoord = RandomFloat( bounds.mins.y, bounds.maxs.y );
	return Vector2f( xCoord, yCoord );
}


//-------------------------------------------------------------------------------------------------
Vector2f UnitVectorFromDegrees( float degrees )
{
	return Vector2f( CosDegrees( degrees ), SinDegrees( degrees ) );
}


//-------------------------------------------------------------------------------------------------
float DistanceBetweenPoints( const Vector2f& position1, const Vector2f& position2 )
{
	return sqrt( DistanceBetweenPointsSquared( position1, position2 ) );
}


//-------------------------------------------------------------------------------------------------
float DistanceBetweenPoints( const Vector3f& position1, const Vector3f& position2 )
{
	return sqrt( DistanceBetweenPointsSquared( position1, position2 ) );
}


//-------------------------------------------------------------------------------------------------
float DistanceBetweenPointsSquared( const Vector2f& position1, const Vector2f& position2 )
{
	float xDistance = position1.x - position2.x;
	float yDistance = position1.y - position2.y;
	return ( xDistance * xDistance ) + ( yDistance * yDistance );
}


//-------------------------------------------------------------------------------------------------
float DistanceBetweenPointsSquared( const Vector3f& position1, const Vector3f& position2 )
{
	float xDistance = position1.x - position2.x;
	float yDistance = position1.y - position2.y;
	float zDistance = position1.z - position2.z;
	return ( xDistance * xDistance ) + ( yDistance * yDistance ) + ( zDistance * zDistance );
}


//-------------------------------------------------------------------------------------------------
bool OverlapDiscs( const Vector2f& center1, float radius1, const Vector2f& center2, float radius2 )
{
	float distanceSquared = DistanceBetweenPointsSquared( center1, center2 );
	float radii = radius1 + radius2;
	float radiiSquared = radii * radii;
	return distanceSquared < radiiSquared;
}


//-------------------------------------------------------------------------------------------------
bool OverlapAABB2s( AABB2f const & box1, AABB2f const box2 )
{
	if( box1.maxs.x < box2.mins.x ) return false;
	if( box1.mins.x > box2.maxs.x ) return false;
	if( box1.maxs.y < box2.mins.y ) return false;
	if( box1.mins.y > box2.maxs.y ) return false;
	return true; // boxes overlap
}


//-------------------------------------------------------------------------------------------------
bool OverlapDiscAndAABB2( Vector2f const & center, float radius, AABB2f const & box )
{
	float left = center.x + radius;
	if( left < box.mins.x )
	{
		return false;
	}
	float right = center.x - radius;
	if( right > box.maxs.x )
	{
		return false;
	}
	float bottom = center.y + radius;
	if( bottom < box.mins.y )
	{
		return false;
	}
	float top = center.y - radius;
	if( top > box.maxs.y )
	{
		return false;
	}
	return true;
}


//-------------------------------------------------------------------------------------------------
float Lerp( float start, float end, float fractionToEnd )
{
	fractionToEnd = Clamp( fractionToEnd, 0.f, 1.f );
	return start * ( 1 - fractionToEnd ) + end * ( fractionToEnd );
}


//-------------------------------------------------------------------------------------------------
Vector2f Lerp( Vector2f const & start, Vector2f const & end, float fractionToEnd )
{
	fractionToEnd = Clamp( fractionToEnd, 0.f, 1.f );
	return start * ( 1 - fractionToEnd ) + end * ( fractionToEnd );
}


//-------------------------------------------------------------------------------------------------
Vector3f Lerp( Vector3f const & start, Vector3f const & end, float fractionToEnd )
{
	fractionToEnd = Clamp( fractionToEnd, 0.f, 1.f );
	return start * ( 1 - fractionToEnd ) + end * ( fractionToEnd );
}


//-------------------------------------------------------------------------------------------------
float LerpDegrees( float start, float end, float fractionToEnd )
{
	fractionToEnd = Clamp( fractionToEnd, 0.f, 1.f );
	float diff = ShortestSignedAngularDistance( start, end );
	return start + diff * fractionToEnd;
}


//-------------------------------------------------------------------------------------------------
//r=right, u=up, f=forward, t=translation
Matrix4f LerpMatrix( Matrix4f const & start, Matrix4f const & end, float fractionToEnd )
{
	Vector3f r0, u0, f0, t0;
	start.GetBasis( &r0, &u0, &f0, &t0 );

	Vector3f r1, u1, f1, t1;
	end.GetBasis( &r1, &u1, &f1, &t1 );

	Vector3f r, u, f, t;
	r = Lerp( r0, r1, fractionToEnd ); //#TODO: Change to slerp
	u = Lerp( u0, u1, fractionToEnd ); //#TODO: Change to slerp
	f = Lerp( f0, f1, fractionToEnd ); //#TODO: Change to slerp
	t = Lerp( t0, t1, fractionToEnd );

	Matrix4f result = Matrix4f::IDENTITY;
	result.SetBasis( r, u, f );
	result.SetTranslation( t );
	return result;
}


//-------------------------------------------------------------------------------------------------
//Return value to [0-1] proportional to range
float RangeMapNormalize( float inputStart, float inputEnd, float inputValue )
{
	ASSERT_RECOVERABLE( inputStart < inputEnd, "Starting value should be less than Ending value." );

	if ( inputValue < inputStart )
		return inputStart;
	if ( inputValue > inputEnd )
		return inputEnd;

	float inputRange = inputEnd - inputStart;
	float normalValue = ( inputValue - inputStart ) / inputRange;

	return normalValue;
}


//-------------------------------------------------------------------------------------------------
float RangeMap( float inputStart, float inputEnd, float inputValue, float outputStart, float outputEnd )
{
	ASSERT_RECOVERABLE( outputStart < outputEnd, "Output Starting value should be less than Ending value." );

	float outputRange = outputEnd - outputStart;
	float outputValue = RangeMapNormalize( inputStart, inputEnd, inputValue )*( outputRange ) +outputStart;
	return outputValue;
}


//-------------------------------------------------------------------------------------------------
float Clamp( float numToClamp, float minNum, float maxNum )
{
	if ( numToClamp < minNum )
		return minNum;
	else if ( numToClamp > maxNum )
		return maxNum;
	return numToClamp;
}


//-------------------------------------------------------------------------------------------------
int Clamp( int numToClamp, int minNum, int maxNum )
{
	if ( numToClamp < minNum )
		return minNum;
	else if ( numToClamp > maxNum )
		return maxNum;
	return numToClamp;
}


//-------------------------------------------------------------------------------------------------
Vector2f Clamp( Vector2f const & vecToClamp, Vector2f const & minVec, Vector2f const & maxVec )
{
	float x = vecToClamp.x;
	if( x < minVec.x )
		x = minVec.x;
	else if( x > maxVec.x )
		x = maxVec.x;

	float y = vecToClamp.y;
	if( y < minVec.y )
		y = minVec.y;
	else if( y > maxVec.y )
		y = maxVec.y;

	return Vector2f( x, y );
}


//-------------------------------------------------------------------------------------------------
float Min( float num1, float num2 )
{
	return num1 < num2 ? ( num1 ) : ( num2 );
}


//-------------------------------------------------------------------------------------------------
int Min( int num1, int num2 )
{
	return num1 < num2 ? ( num1 ) : ( num2 );
}


//-------------------------------------------------------------------------------------------------
size_t Min( size_t num1, size_t num2 )
{
	return num1 < num2 ? ( num1 ) : ( num2 );
}


//-------------------------------------------------------------------------------------------------
uint16_t Min( uint16_t num1, uint16_t num2 )
{
	return num1 < num2 ? ( num1 ) : ( num2 );
}


//-------------------------------------------------------------------------------------------------
float Max( float num1, float num2 )
{
	return num1 > num2 ? ( num1 ) : ( num2 );
}


//-------------------------------------------------------------------------------------------------
int Max( int num1, int num2 )
{
	return num1 > num2 ? ( num1 ) : ( num2 );
}


//-------------------------------------------------------------------------------------------------
size_t Max( size_t num1, size_t num2 )
{
	return num1 > num2 ? ( num1 ) : ( num2 );
}


//-------------------------------------------------------------------------------------------------
int RoundToNearestInt( float numToRound )
{
	return ( int ) round( numToRound );
}


//-----------------------------------------------------------------------------------------------
// A replacement for floor(), about 3x faster on my machine.
// Reliable within [-2 billion, +2 billion] or so.  I think.
// Credit: Squirrel Eiserloh
float FastFloor( float f )
{
	if ( f >= 0.f )
		return ( float ) ( int ) f;
	else
	{
		float f2 = ( float ) ( int ) f;
		if ( f == f2 )
			return f2;
		else
			return f2 - 1.f;
	}
}


//-----------------------------------------------------------------------------------------------
// A replacement for (int) floor(), about 3x faster on my machine
// Reliable within the range of int.  I think.
// Credit: Squirrel Eiserloh
int FastFloorToInt( float f )
{
	if ( f >= 0.f )
		return ( int ) f;
	else
	{
		int i = ( int ) f;
		float f2 = ( float ) i;
		if ( f == f2 )
			return i;
		else
			return i - 1;
	}
}


//-------------------------------------------------------------------------------------------------
bool GreaterThanCycle( uint16_t num1, uint16_t num2 )
{
	uint16_t halfMax = ( (uint16_t) -1 ) / 2; //should be 0x7fff
	uint16_t diff = num1 - num2;
	return ( diff > 0 ) && ( diff < halfMax );
}


//-------------------------------------------------------------------------------------------------
uint8_t CompressFloat32ToUint8( float actual, float min, float max )
{
	float zeroToOne = RangeMapNormalize( min, max, actual );
	return (uint8_t) ( (float) MAX_UINT8 * zeroToOne );
}


//-------------------------------------------------------------------------------------------------
float DecompressUint8ToFloat32( uint8_t compressed, float min, float max )
{
	float zeroToOne = (float) compressed / (float) MAX_UINT8;
	float range = max - min;
	return min + range * zeroToOne;
}


//-------------------------------------------------------------------------------------------------
uint16_t CompressFloat32ToUint16( float actual, float min, float max )
{
	float zeroToOne = RangeMapNormalize( min, max, actual );
	return (uint16_t) ( (float) MAX_UINT16 * zeroToOne );
}


//-------------------------------------------------------------------------------------------------
float DecompressUint16ToFloat32( uint16_t compressed, float min, float max )
{
	float zeroToOne = (float) compressed / (float) MAX_UINT16;
	float range = max - min;
	return min + range * zeroToOne;
}


//-------------------------------------------------------------------------------------------------
uint32_t CompressVector2fToUint32( Vector2f const & actual, Vector2f const & min, Vector2f const & max )
{
	uint16_t xCompressed = CompressFloat32ToUint16( actual.x, min.x, max.x );
	uint16_t yCompressed = CompressFloat32ToUint16( actual.y, min.y, max.y );
	uint32_t vectorCompressed = (uint32_t)yCompressed;
	vectorCompressed = vectorCompressed << 16; //pack both uint16_t into one uint32_t
	vectorCompressed = vectorCompressed | xCompressed;
	return vectorCompressed;
}


//-------------------------------------------------------------------------------------------------
Vector2f DecompressUint32ToVector2f( uint32_t compressed, Vector2f const & min, Vector2f const & max )
{
	uint32_t xMask = 0x0000FFFF;
	uint16_t xCompressed = (uint16_t) ( compressed & xMask );
	compressed = compressed >> 16;
	uint16_t yCompressed = (uint16_t) ( compressed );
	float x = DecompressUint16ToFloat32( xCompressed, min.x, max.x );
	float y = DecompressUint16ToFloat32( yCompressed, min.y, max.y );
	return Vector2f( x, y );
}


//-------------------------------------------------------------------------------------------------
//Returns 3x^2 - 2x^3
float SmoothStep( float inputZeroToOne )
{
	ASSERT_RECOVERABLE( inputZeroToOne >= 0.0f && inputZeroToOne <= 1.0f, "Step only gives reasonable results for values 0 to 1." );

	float inputPow3 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne );
	float inputPow2 = ( inputZeroToOne * inputZeroToOne );
	return ( 3.f * inputPow2 ) - ( 2.f * inputPow3 );
}


//-------------------------------------------------------------------------------------------------
//Returns 2x^3 - 3x^2 + 2x
float SmoothStepInverse( float inputZeroToOne )
{
	ASSERT_RECOVERABLE( inputZeroToOne >= 0.0f && inputZeroToOne <= 1.0f, "Step only gives reasonable results for values 0 to 1." );

	float inputPow3 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne );
	float inputPow2 = ( inputZeroToOne * inputZeroToOne );
	return ( 2.f * inputPow3 ) - ( 3.f * inputPow2 ) + ( 2.f * inputZeroToOne );
}


//-------------------------------------------------------------------------------------------------
//Returns 6x^5 - 15x^4 + 10x^3
float SmootherStep( float inputZeroToOne )
{
	ASSERT_RECOVERABLE( inputZeroToOne >= 0.0f && inputZeroToOne <= 1.0f, "Step only gives reasonable results for values 0 to 1." );

	float inputPow5 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne );
	float inputPow4 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne );
	float inputPow3 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne );
	return ( 6.f * inputPow5 ) - ( 15.f * inputPow4 ) + ( 10.f * inputPow3 );
}


//-------------------------------------------------------------------------------------------------
//Returns -20x^7 + 70x^6 - 84x^5 + 35x^4
float SmoothestStep( float inputZeroToOne )
{
	ASSERT_RECOVERABLE( inputZeroToOne >= 0.0f && inputZeroToOne <= 1.0f, "Step only gives reasonable results for values 0 to 1." );

	float inputPow7 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne );
	float inputPow6 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne );
	float inputPow5 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne );
	float inputPow4 = ( inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne );
	return ( -20.f * inputPow7 ) + ( 70.f * inputPow6 ) - ( 84.f * inputPow5 ) + ( 35.f * inputPow4 );
}

//-------------------------------------------------------------------------------------------------
float DotProduct( const Vector2f& vector1, const Vector2f& vector2 )
{
	float xProducts = vector1.x * vector2.x;
	float yProducts = vector1.y * vector2.y;
	return xProducts + yProducts;
}


//-------------------------------------------------------------------------------------------------
float DotProduct( const Vector3f& vector1, const Vector3f& vector2 )
{
	float xProducts = vector1.x * vector2.x;
	float yProducts = vector1.y * vector2.y;
	float zProducts = vector1.z * vector2.z;
	return xProducts + yProducts + zProducts;
}


//-------------------------------------------------------------------------------------------------
float DotProduct( const Vector4f& vector1, const Vector4f& vector2 )
{
	float xProducts = vector1.x * vector2.x;
	float yProducts = vector1.y * vector2.y;
	float zProducts = vector1.z * vector2.z;
	float wProducts = vector1.w * vector2.w;
	return xProducts + yProducts + zProducts + wProducts;
}


//-------------------------------------------------------------------------------------------------
const Vector3f CrossProduct( const Vector3f& vector1, const Vector3f& vector2 )
{
	float x = vector1.y * vector2.z - vector2.y * vector1.z;
	float y = vector1.z * vector2.x - vector2.z * vector1.x;
	float z = vector1.x * vector2.y - vector2.x * vector1.y;
	return Vector3f( x, y, z );
}


//-----------------------------------------------------------------------------------------------
float GetPseudoRandomNoise1D( int position )
{
	const float ONE_OVER_MAX_POSITIVE_INT = ( 1.f / 2147483648.f );
	int bits = ( position << 13 ) ^ position;
	int pseudoRandomPositiveInt = ( bits * ( ( bits * bits * 15731 ) + 789221 ) + 1376312589 ) & 0x7fffffff;
	float pseudoRandomFloatZeroToOne = ONE_OVER_MAX_POSITIVE_INT * ( float ) pseudoRandomPositiveInt;
	return pseudoRandomFloatZeroToOne;
}


//-----------------------------------------------------------------------------------------------
float GetPseudoRandomNoise2D( int positionX, int positionY )
{
	const float ONE_OVER_MAX_POSITIVE_INT = ( 1.f / 2147483648.f );
	int position = positionX + ( positionY * 57 );
	int bits = ( position << 13 ) ^ position;
	int pseudoRandomPositiveInt = ( bits * ( ( bits * bits * 15731 ) + 789221 ) + 1376312589 ) & 0x7fffffff;
	float pseudoRandomFloatZeroToOne = ONE_OVER_MAX_POSITIVE_INT * ( float ) pseudoRandomPositiveInt;
	return pseudoRandomFloatZeroToOne;
}


//-----------------------------------------------------------------------------------------------
float GetPseudoRandomNoise3D( int positionX, int positionY, int positionZ )
{
	const float ONE_OVER_MAX_POSITIVE_INT = ( 1.f / 2147483648.f );
	int seed = positionX + ( positionY * 57 ) + ( positionZ * 3023 );
	int bits = ( seed << 13 ) ^ seed;
	int pseudoRandomPositiveInt = ( bits * ( ( bits * bits * 15731 ) + 789221 ) + 1376312589 ) & 0x7fffffff;
	float pseudoRandomFloatZeroToOne = ONE_OVER_MAX_POSITIVE_INT * ( float ) pseudoRandomPositiveInt;
	return pseudoRandomFloatZeroToOne;
}


//-----------------------------------------------------------------------------------------------
Vector2f GetPseudoRandomNoiseDirection2D( int xPosition, int yPosition )
{
	const float ONE_OVER_MAX_POSITIVE_INT = ( 1.f / 2147483648.f );
	const float SCALE_FACTOR = ONE_OVER_MAX_POSITIVE_INT * 360.f;
	int position = xPosition + ( yPosition * 57 );
	int bits = ( position << 13 ) ^ position;
	int pseudoRandomPositiveInt = ( bits * ( ( bits * bits * 15731 ) + 789221 ) + 1376312589 ) & 0x7fffffff;
	float pseudoRandomDegrees = SCALE_FACTOR * ( float ) pseudoRandomPositiveInt;

	// #TODO: Rewrite this to use the randomized int to look up Vector2 from a (small) cos/sin
	//	table; vectors don't need to be super-precise, and we certainly don't want to pay full
	//	price for cos/sin if this is merely going to be used for, say, Perlin noise gradients.
	//	Note that cos/sin are typically fast on GPUs so this can probably stand in shader code.
	return Vector2f( CosDegrees( pseudoRandomDegrees ), SinDegrees( pseudoRandomDegrees ) );
}


//-------------------------------------------------------------------------------------------------
bool IsBitfieldSet( uint16_t bitFlags, size_t bitMask )
{
	return ( bitFlags & bitMask ) != 0;
}


//-------------------------------------------------------------------------------------------------
bool IsBitfieldSet( uint8_t bitFlags, size_t bitMask )
{
	return ( bitFlags & bitMask ) != 0;
}


//-------------------------------------------------------------------------------------------------
bool IsBitSet( uint16_t bitFlags, size_t bitIndex )
{
	return ( bitFlags & BIT( bitIndex ) ) != 0;
}


//-------------------------------------------------------------------------------------------------
bool IsBitSet( uint8_t bitFlags, size_t bitIndex )
{
	return ( bitFlags & BIT( bitIndex ) ) != 0;
}


//-------------------------------------------------------------------------------------------------
void SetBit( uint16_t * out_bitField, size_t bitToSet )
{
	*out_bitField |= BIT( bitToSet );
}


//-------------------------------------------------------------------------------------------------
void SetBit( uint8_t * out_bitField, size_t bitToSet )
{
	*out_bitField |= BIT( bitToSet );
}


//-------------------------------------------------------------------------------------------------
size_t HashMemory( void const * memory, size_t const memorySize )
{
	byte_t const * bytes = (byte_t const *) memory;
	size_t hash = 0xFFFFFFFF;

	size_t memIndex = 0;
	size_t repeat;
	size_t byte;
	size_t mask;

	for( memIndex = 0; memIndex < memorySize; ++memIndex )
	{
		// Get next byte.
		byte = bytes[memIndex];
		hash = hash ^ byte;

		// Do eight times.
		for( repeat = 0; repeat < 8; ++repeat )
		{
			//added 0x00000000 infront of minus because it gave me a warning trying to use negative on something unsigned.
			mask = 0x00000000 - ( hash & 1 );
			hash = ( hash >> 1 ) ^ ( 0xEDB88320 & mask );
		}
		memIndex += 1;
	}
	return ~hash;
}