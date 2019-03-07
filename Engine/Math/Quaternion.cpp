#include "Engine/Math/Quaternion.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/Utils/MathUtils.hpp"


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::ZERO( 1.f, Vector3f::ZERO );


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion( )
	: w( 1.0 )
	, x( 0.0 )
	, y( 0.0 )
	, z( 0.0 )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion( double real, Vector3f const &img )
	: w( real )
	, x( ( double ) img.x )
	, y( ( double ) img.y )
	, z( ( double ) img.z )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion( double realw, double imgx, double imgy, double imgz )
	: w( realw )
	, x( imgx )
	, y( imgy )
	, z( imgz )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion( Euler const & eulerAngles )
{
	Matrix4f mat( eulerAngles );
	//TODO: Finish implementation
}


//-------------------------------------------------------------------------------------------------
Quaternion::Quaternion( Matrix4f const & )
{
	//#TODO: Implement
}


//-------------------------------------------------------------------------------------------------
Vector3f Quaternion::GetComplex( ) const
{
	return Vector3f( ( float ) x, ( float ) y, ( float ) z );
}


//-------------------------------------------------------------------------------------------------
double Quaternion::GetReal( ) const
{
	return w;
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::GetConjugate( void ) const
{
	return Quaternion( GetReal( ), -GetComplex( ) );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::Inverse( ) const
{
	return GetConjugate( ) / norm( );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::Product( Quaternion const & rhs ) const
{
	double prodX = y*rhs.z - z*rhs.y + x*rhs.w + w*rhs.x;
	double prodY = z*rhs.x - x*rhs.z + y*rhs.w + w*rhs.y;
	double prodZ = x*rhs.y - y*rhs.x + z*rhs.w + w*rhs.z;
	double prodW = w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z;

	return Quaternion( prodW, Vector3f( ( float ) prodX, ( float ) prodY, ( float ) prodZ ) );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::operator*( Quaternion const & rhs ) const
{
	return Product( rhs );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::operator*( double s ) const
{
	return Quaternion( GetReal( )*s, GetComplex( )*( float ) s );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::operator+( Quaternion const & rhs ) const
{
	double sumW = w + rhs.w;
	double sumX = x + rhs.x;
	double sumY = y + rhs.y;
	double sumZ = z + rhs.z;

	return Quaternion( sumW, sumX, sumY, sumZ );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::operator-( Quaternion const & rhs ) const
{
	double subW = w - rhs.w;
	double subX = x - rhs.x;
	double subY = y - rhs.y;
	double subZ = z - rhs.z;

	return Quaternion( subW, subX, subY, subZ );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::operator-( ) const
{
	double prodX = -x;
	double prodY = -y;
	double prodZ = -z;
	double prodW = -w;

	return Quaternion( prodW, prodX, prodY, prodZ );
}


//-------------------------------------------------------------------------------------------------
Quaternion Quaternion::operator/( double s ) const
{
	ASSERT_RECOVERABLE( s != 0.0, "Can't divide by zero" );
	double oneOverS = 1.0 / s;
	return Quaternion( GetReal( ) * oneOverS, GetComplex( ) * ( float ) oneOverS );
}


//-------------------------------------------------------------------------------------------------
double Quaternion::normSquared( ) const
{
	return x * x + y * y + z * z + w * w;
}


//-------------------------------------------------------------------------------------------------
double Quaternion::norm( ) const
{
	return sqrt( normSquared( ) );
}


//-------------------------------------------------------------------------------------------------
Matrix4f Quaternion::GetRotationMatrix( ) const
{
	float mat[16] =
	{
		( float ) ( 1.0 - 2.0 * y * y - 2 * z * z ),	( float ) ( 2.0 * x * y - 2.0 * z * w ),		( float ) ( 2.0 * x * z + 2.0 * y * w ),		0.0f,
		( float ) ( 2.0 * x * y + 2.0 * z * w ),		( float ) ( 1.0 - 2.0 * x * x - 2 * z * z ),	( float ) ( 2.0 * y * z - 2 * x * w ),			0.0f,
		( float ) ( 2.0 * x * z - 2.0 * y * w ),		( float ) ( 2.0 * y * z + 2.0 * x * w ),		( float ) ( 1.0 - 2.0 * x * x - 2 * y * y ),	0.0f,
		0.0f,											0.0f,											0.0f,											1.0f
	};
	return Matrix4f( mat );
}


//-------------------------------------------------------------------------------------------------
void Quaternion::MakeEuler( Euler const & euler )
{
	double c1 = CosDegrees( euler.m_yawDegreesAboutY	* 0.5f );
	double c2 = CosDegrees( euler.m_pitchDegreesAboutX	* 0.5f );
	double c3 = CosDegrees( euler.m_rollDegreesAboutZ	* 0.5f );
	double s1 = SinDegrees( euler.m_yawDegreesAboutY	* 0.5f );
	double s2 = SinDegrees( euler.m_pitchDegreesAboutX	* 0.5f );
	double s3 = SinDegrees( euler.m_rollDegreesAboutZ	* 0.5f );

	x = c1 * c2 * s3 - s1 * s2 * c3;
	y = c1 * s2 * c3 + s1 * c2 * s3;
	z = s1 * c2 * c3 - c1 * s2 * s3;
	w = c1 * c2 * c3 + s1 * s2 * s3;
}


//-------------------------------------------------------------------------------------------------
Euler Quaternion::GetEuler( ) const
{
	Euler euler = Euler( 0.f, 0.f, 0.f );
	const static double PI_OVER_2 = ( double ) PI * 0.5;
	const static double EPSILON = 1e-10;
	double sqw, sqx, sqy, sqz;

	// quick conversion to Euler angles to give tilt to user
	sqw = w * w;
	sqx = x * x;
	sqy = y * y;
	sqz = z * z;

	euler.m_pitchDegreesAboutX = RadToDeg( ( float ) asin( 2.0 * ( w * y - x * z ) ) );
	if ( PI_OVER_2 - fabs( euler.m_pitchDegreesAboutX ) > EPSILON )
	{
		euler.m_yawDegreesAboutY = Atan2Degrees( ( float ) ( 2.0 * ( x * y + w * z ) ), ( float ) ( sqx - sqy - sqz + sqw ) );
		euler.m_rollDegreesAboutZ = Atan2Degrees( ( float ) ( 2.0 * ( w * x + y * z ) ), ( float ) ( sqw - sqx - sqy + sqz ) );
	}
	else
	{
		// compute heading from local 'down' vector
		euler.m_yawDegreesAboutY = Atan2Degrees( ( float ) ( 2 * y * z - 2 * x * w ), ( float ) ( 2 * x * z + 2 * y * w ) );
		euler.m_rollDegreesAboutZ = 0.0f;

		// If facing down, reverse yaw
		if ( euler.m_pitchDegreesAboutX < 0 )
			euler.m_yawDegreesAboutY = RadToDeg( PI ) - euler.m_yawDegreesAboutY;
	}
	return euler;
}