#include "Engine/Math/Matrix4f.hpp"

#include "Engine/Utils/MathUtils.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
STATIC Matrix4f const Matrix4f::IDENTITY = Matrix4f
(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
);


//-------------------------------------------------------------------------------------------------
STATIC Matrix4f Matrix4f::GetEngineBasis( )
{
	return Matrix4f
	(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
}


//-------------------------------------------------------------------------------------------------
STATIC Matrix4f Matrix4f::Orthonormal( float width, float height, float nz /*= -1.f*/, float fz /*= 1.f*/ )
{
	Matrix4f orthonormal;
	orthonormal.MakeOrthonormal( width, height, nz, fz );
	return orthonormal;
}


//-------------------------------------------------------------------------------------------------
STATIC Matrix4f Matrix4f::View( Vector3f const & position /*= Vector3f::ZERO*/, Euler const & orientation /*= Euler::ZERO */ )
{
	Matrix4f view;
	view.MakeView( position, orientation );
	return view;
}


//-------------------------------------------------------------------------------------------------
Matrix4f::Matrix4f( )
	: Matrix4f( IDENTITY )
{
}


//-------------------------------------------------------------------------------------------------
Matrix4f::Matrix4f
(
	float r1c1, float r1c2, float r1c3, float r1c4,
	float r2c1, float r2c2, float r2c3, float r2c4,
	float r3c1, float r3c2, float r3c3, float r3c4,
	float r4c1, float r4c2, float r4c3, float r4c4
)
{
	m_data[0] = r1c1;
	m_data[1] = r1c2;
	m_data[2] = r1c3;
	m_data[3] = r1c4;
	m_data[4] = r2c1;
	m_data[5] = r2c2;
	m_data[6] = r2c3;
	m_data[7] = r2c4;
	m_data[8] = r3c1;
	m_data[9] = r3c2;
	m_data[10] = r3c3;
	m_data[11] = r3c4;
	m_data[12] = r4c1;
	m_data[13] = r4c2;
	m_data[14] = r4c3;
	m_data[15] = r4c4;
}


//-------------------------------------------------------------------------------------------------
Matrix4f::Matrix4f( float const * src )
{
	memcpy( &m_data[0], &src[0], sizeof( float ) * 16 );
}


//-------------------------------------------------------------------------------------------------
Matrix4f::Matrix4f( Matrix4f const & src )
{
	memcpy( &m_data[0], &(src.m_data)[0], sizeof( float ) * 16 );
}


//-------------------------------------------------------------------------------------------------
Matrix4f::Matrix4f( Euler const & eulerAngles )
{
	Matrix4f newMatrix( IDENTITY );
	newMatrix.MakeView( Vector3f::ZERO, eulerAngles );

	memcpy( &m_data[0], &newMatrix.m_data[0], sizeof( float ) * 16 );
}


//-------------------------------------------------------------------------------------------------
Matrix4f::Matrix4f( Vector4f const & row1, Vector4f const & row2, Vector4f const & row3, Vector4f const & row4 )
{
	memcpy( &m_data[0], &row1, sizeof( float ) * 4 );
	memcpy( &m_data[4], &row2, sizeof( float ) * 4 );
	memcpy( &m_data[8], &row3, sizeof( float ) * 4 );
	memcpy( &m_data[12], &row4, sizeof( float ) * 4 );
}


//-------------------------------------------------------------------------------------------------
Matrix4f::~Matrix4f( )
{
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::operator=( float const * rhs )
{
	memcpy( &m_data[0], &rhs[0], sizeof( float ) * 16 );
}


//-------------------------------------------------------------------------------------------------
Matrix4f const operator*( Matrix4f const &lhs, Matrix4f const &rhs )
{
	float result[16];
	result[0] = lhs.m_data[0] * rhs.m_data[0] + lhs.m_data[1] * rhs.m_data[4] + lhs.m_data[2] * rhs.m_data[8] + lhs.m_data[3] * rhs.m_data[12];
	result[1] = lhs.m_data[0] * rhs.m_data[1] + lhs.m_data[1] * rhs.m_data[5] + lhs.m_data[2] * rhs.m_data[9] + lhs.m_data[3] * rhs.m_data[13];
	result[2] = lhs.m_data[0] * rhs.m_data[2] + lhs.m_data[1] * rhs.m_data[6] + lhs.m_data[2] * rhs.m_data[10] + lhs.m_data[3] * rhs.m_data[14];
	result[3] = lhs.m_data[0] * rhs.m_data[3] + lhs.m_data[1] * rhs.m_data[7] + lhs.m_data[2] * rhs.m_data[11] + lhs.m_data[3] * rhs.m_data[15];
	
	result[4] = lhs.m_data[4] * rhs.m_data[0] + lhs.m_data[5] * rhs.m_data[4] + lhs.m_data[6] * rhs.m_data[8] + lhs.m_data[7] * rhs.m_data[12];
	result[5] = lhs.m_data[4] * rhs.m_data[1] + lhs.m_data[5] * rhs.m_data[5] + lhs.m_data[6] * rhs.m_data[9] + lhs.m_data[7] * rhs.m_data[13];
	result[6] = lhs.m_data[4] * rhs.m_data[2] + lhs.m_data[5] * rhs.m_data[6] + lhs.m_data[6] * rhs.m_data[10] + lhs.m_data[7] * rhs.m_data[14];
	result[7] = lhs.m_data[4] * rhs.m_data[3] + lhs.m_data[5] * rhs.m_data[7] + lhs.m_data[6] * rhs.m_data[11] + lhs.m_data[7] * rhs.m_data[15];
	
	result[8] = lhs.m_data[8] * rhs.m_data[0] + lhs.m_data[9] * rhs.m_data[4] + lhs.m_data[10] * rhs.m_data[8] + lhs.m_data[11] * rhs.m_data[12];
	result[9] = lhs.m_data[8] * rhs.m_data[1] + lhs.m_data[9] * rhs.m_data[5] + lhs.m_data[10] * rhs.m_data[9] + lhs.m_data[11] * rhs.m_data[13];
	result[10] = lhs.m_data[8] * rhs.m_data[2] + lhs.m_data[9] * rhs.m_data[6] + lhs.m_data[10] * rhs.m_data[10] + lhs.m_data[11] * rhs.m_data[14];
	result[11] = lhs.m_data[8] * rhs.m_data[3] + lhs.m_data[9] * rhs.m_data[7] + lhs.m_data[10] * rhs.m_data[11] + lhs.m_data[11] * rhs.m_data[15];
	
	result[12] = lhs.m_data[12] * rhs.m_data[0] + lhs.m_data[13] * rhs.m_data[4] + lhs.m_data[14] * rhs.m_data[8] + lhs.m_data[15] * rhs.m_data[12];
	result[13] = lhs.m_data[12] * rhs.m_data[1] + lhs.m_data[13] * rhs.m_data[5] + lhs.m_data[14] * rhs.m_data[9] + lhs.m_data[15] * rhs.m_data[13];
	result[14] = lhs.m_data[12] * rhs.m_data[2] + lhs.m_data[13] * rhs.m_data[6] + lhs.m_data[14] * rhs.m_data[10] + lhs.m_data[15] * rhs.m_data[14];
	result[15] = lhs.m_data[12] * rhs.m_data[3] + lhs.m_data[13] * rhs.m_data[7] + lhs.m_data[14] * rhs.m_data[11] + lhs.m_data[15] * rhs.m_data[15];
	return Matrix4f( result );
}


//-------------------------------------------------------------------------------------------------
Matrix4f const operator*( float lhs, Matrix4f const &rhs )
{
	float result[16];
	result[0] = lhs * rhs.m_data[0];
	result[1] = lhs * rhs.m_data[1];
	result[2] = lhs * rhs.m_data[2];
	result[3] = lhs * rhs.m_data[3];

	result[4] = lhs * rhs.m_data[4];
	result[5] = lhs * rhs.m_data[5];
	result[6] = lhs * rhs.m_data[6];
	result[7] = lhs * rhs.m_data[7];

	result[8] = lhs * rhs.m_data[8];
	result[9] = lhs * rhs.m_data[9];
	result[10] =lhs * rhs.m_data[10];
	result[11] =lhs * rhs.m_data[11];

	result[12] =lhs * rhs.m_data[12];
	result[13] =lhs * rhs.m_data[13];
	result[14] =lhs * rhs.m_data[14];
	result[15] =lhs * rhs.m_data[15];
	return Matrix4f( result );
}


//-------------------------------------------------------------------------------------------------
Vector4f const operator*( Vector4f const &lhs, Matrix4f const &rhs ) //#TODO: Reverse
{
	Vector4f result;

	result.x = lhs.x * rhs.m_data[0] + lhs.y * rhs.m_data[4] + lhs.z * rhs.m_data[8] + lhs.w * rhs.m_data[12];
	result.y = lhs.x * rhs.m_data[1] + lhs.y * rhs.m_data[5] + lhs.z * rhs.m_data[9] + lhs.w * rhs.m_data[13];
	result.z = lhs.x * rhs.m_data[2] + lhs.y * rhs.m_data[6] + lhs.z * rhs.m_data[10] + lhs.w * rhs.m_data[14];
	result.w = lhs.x * rhs.m_data[3] + lhs.y * rhs.m_data[7] + lhs.z * rhs.m_data[11] + lhs.w * rhs.m_data[15];

	return result;
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::Transpose( )
{
	std::swap( m_data[1], m_data[4] );
	std::swap( m_data[2], m_data[8] );
	std::swap( m_data[6], m_data[9] );
	std::swap( m_data[3], m_data[12] );
	std::swap( m_data[7], m_data[13] );
	std::swap( m_data[11], m_data[14] );
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::Orthogonalize( )
{
	Vector3f firstRow;
	firstRow.x = m_data[0];
	firstRow.y = m_data[1];
	firstRow.z = m_data[2];
	firstRow.Normalize( ); //Needed

	Vector3f secondRow;
	secondRow.x = m_data[4];
	secondRow.y = m_data[5];
	secondRow.z = m_data[6];

	Vector3f thirdRow = CrossProduct( firstRow, secondRow );
	thirdRow.Normalize( ); //Needed

	secondRow = CrossProduct( thirdRow, firstRow );
	//Not needed

	m_data[0] = firstRow.x;
	m_data[1] = firstRow.y;
	m_data[2] = firstRow.z;

	m_data[4] = secondRow.x;
	m_data[5] = secondRow.y;
	m_data[6] = secondRow.z;

	m_data[8] = thirdRow.x;
	m_data[9] = thirdRow.y;
	m_data[10] = thirdRow.z;
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::InvertOrthonormal( )
{
	//(R*T)^-1 = T^-1 * R^-1

	//Rotation
	Matrix4f rotation( &this->m_data[0] );
	rotation.m_data[12] = 0.f;
	rotation.m_data[13] = 0.f;
	rotation.m_data[14] = 0.f;

	//Rotation^-1
	std::swap( rotation.m_data[1], rotation.m_data[4] );
	std::swap( rotation.m_data[2], rotation.m_data[8] );
	std::swap( rotation.m_data[6], rotation.m_data[9] );

	//Translation^-1
	Matrix4f translation( IDENTITY );
	translation.SetTranslation( -m_data[12], -m_data[13], -m_data[14] );

	Matrix4f const inverse = translation * rotation;
	memcpy( &m_data[0], &inverse.m_data[0], sizeof( float ) * 16 );
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::MakeOrthonormal( float width, float height, float nz, float fz )
{
	float sz = 1.0f / ( fz - nz );

	float const values[] = {
		2.0f / width,	0.0f,			0.0f,				0.0f,
		0.0f,			2.0f / height,  0.0f,				0.0f,
		0.0f,			0.0f,			2.0f * sz,			0.0f,
		0.0f,			0.0f,			-( fz + nz ) * sz,  1.0f,
	};

	memcpy( &m_data[0], &values[0], sizeof( values ) );
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::MakePerspective( float fovDegreesY, float aspect, float nearZ, float farZ )
{
	ASSERT_RECOVERABLE( nearZ > 0.f, "Near plane must be greater than 0.f" );
	float rads = DegToRad( fovDegreesY );
	float size = 1.0f / tan( rads / 2.0f );

	float w = size;
	float h = size;
	if ( aspect > 1.0f )
	{
		w /= aspect;
	}
	else
	{
		h *= aspect;
	}

	float q = 1.0f / ( farZ - nearZ );

	float const values[] =
	{
		w,			0.f,		0.f,						0.f,
		0.f,		h,			0.f,						0.f,
		0.f,		0.f,		( farZ + nearZ ) * q,		1.f,
		0.f,		0.f,		-2.f * nearZ * farZ * q,	0.f
	};

	memcpy( &m_data[0], &values[0], sizeof( values ) );
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::MakeLookAt( Vector3f const &position, Vector3f const &target, Vector3f const &worldUpVector /*= Vector3( 0.f, 0.f, 1.f ) */ )
{
	Vector3f forwardVector = target - position;
	forwardVector.Normalize( );
	Vector3f wup = worldUpVector;
	wup.Normalize( );
	Vector3f const rightVector = CrossProduct( forwardVector, wup );
	Vector3f const upVector = CrossProduct( rightVector, forwardVector ); //Already normalized

	SetBasis( rightVector, upVector, forwardVector );
	SetTranslation( position.x, position.z, position.y );
}


//-------------------------------------------------------------------------------------------------
//#TODO: FIX FOR MY ENGINE
void Matrix4f::MakeView( float yawDegrees, float pitchDegrees, float rollDegrees, Vector3f const & position )
{
	Euler orientation( yawDegrees, pitchDegrees, rollDegrees );
	MakeView( position, orientation );
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::MakeView( Vector3f const & position, Euler const & orientation )
{
	SetRotation( orientation );
	SetTranslation( position );
}


//-------------------------------------------------------------------------------------------------
Vector4f const Matrix4f::GetRow( int rowIndex ) const
{
	Vector4f result;

	result.x = m_data[rowIndex * 4 + 0];
	result.y = m_data[rowIndex * 4 + 1];
	result.z = m_data[rowIndex * 4 + 2];
	result.w = m_data[rowIndex * 4 + 3];

	return result;
}


//-------------------------------------------------------------------------------------------------
Vector4f const Matrix4f::GetCol( int colIndex ) const
{
	Vector4f result;

	result.x = m_data[0 + colIndex];
	result.y = m_data[4 + colIndex];
	result.z = m_data[8 + colIndex];
	result.w = m_data[12 + colIndex];

	return result;
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::GetBasis( Vector3f * out_rightVector, Vector3f * out_upVector, Vector3f * out_forwardVector, Vector3f * out_translation ) const
{
	*out_rightVector = GetRow( 0 ).XYZ( );
	*out_upVector = GetRow( 1 ).XYZ( );
	*out_forwardVector = GetRow( 2 ).XYZ( );
	*out_translation = GetRow( 3 ).XYZ( );
}


//-------------------------------------------------------------------------------------------------
Vector3f Matrix4f::GetForward( ) const
{
	Vector4f const row = GetRow( 2 );
	return row.XYZ( );
}


//-------------------------------------------------------------------------------------------------
Matrix4f Matrix4f::GetInverseOrthonormal( ) const
{
	Matrix4f copy( *this );
	copy.InvertOrthonormal( );
	return copy;
}


//-------------------------------------------------------------------------------------------------
Vector3f Matrix4f::GetWorldPositionFromViewNoScale( ) const
{
	Vector3f rightVec;
	Vector3f upVector;
	Vector3f forwardVector;
	Vector3f posVector;
	GetBasis( &rightVec, &upVector, &forwardVector, &posVector );
	Matrix4f rotMat = Matrix4f( Vector4f( rightVec, 0.f ), Vector4f( upVector, 0.f ), Vector4f( forwardVector, 0.f ), Vector4f( 0.f, 0.f, 0.f, 1.f ) );
	rotMat.InvertOrthonormal( );
	Vector3f retVec = ( Vector4f( -posVector, 0.f ) * rotMat ).XYZ( );
	return retVec;
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::SetForward( Vector3f const &forward )
{
	m_data[8] = forward.x;
	m_data[9] = forward.y;
	m_data[10] = forward.z;
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::SetTranslation( Vector3f const &position )
{
	m_data[12] = position.x;
	m_data[13] = position.y;
	m_data[14] = position.z;
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::SetTranslation( float setX, float setY, float setZ )
{
	m_data[12] = setX;
	m_data[13] = setY;
	m_data[14] = setZ;
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::SetRotation( Euler const & orientation )
{
	// pitch = x
	// yaw = y
	// roll = z

	float sx = SinDegrees( orientation.m_pitchDegreesAboutX );
	float cx = CosDegrees( orientation.m_pitchDegreesAboutX );

	float sy = SinDegrees( orientation.m_yawDegreesAboutY );
	float cy = CosDegrees( orientation.m_yawDegreesAboutY );

	float sz = SinDegrees( orientation.m_rollDegreesAboutZ );
	float cz = CosDegrees( orientation.m_rollDegreesAboutZ );

	Vector3f const right( cy*cz + sx*sy*sz, -cx*sz, -cz*sy + cy*sx*sz );
	Vector3f const up( cy*sz - cz*sx*sy, cx*cz, -sy*sz - cy*cz*sx );
	Vector3f const forward( -cx*sy, -sx, -cx*cy );

	SetBasis( right, up, forward );
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::SetBasis( Vector3f const &rightVector, Vector3f const &upVector, Vector3f const &forwardVector )
{
	float const values[] =
	{
		rightVector.x,		rightVector.y,		rightVector.z,		0.0f,
		upVector.x,			upVector.y,			upVector.z,			0.0f,
		forwardVector.x,	forwardVector.y,	forwardVector.z,	0.0f,
		0.0f,				0.0f,				0.0f,				1.0f,
	};

	memcpy( &m_data[0], &values[0], sizeof( values ) );
}


//-------------------------------------------------------------------------------------------------
void Matrix4f::SetScale( Vector3f const &scaleVector )
{
	m_data[0] = scaleVector.x;
	m_data[5] = scaleVector.y;
	m_data[10] = scaleVector.z;
}


//-------------------------------------------------------------------------------------------------
bool Matrix4f::operator==( Matrix4f const & rhs ) const
{
	for ( int index = 0; index < 16; ++index )
	{
		if ( m_data[index] != rhs.m_data[index] )
			return false;
	}
	return true;
}

/*
void MatrixMakeProjOrthogonal( mat44_fl *mat, float nx, float fx, float ny, float fy, float nz, float fz )
{
float sx = 1.0f / (fx - nx);
float sy = 1.0f / (fy - ny);
float sz = 1.0f / (fz - nz);

float const values[] = {
2.0f * sx,  0.0f,       0.0f,       -(fx + nx) * sx,
0.0f,       2.0f * sy,  0.0f,       -(fy + ny) * sy,
0.0f,       0.0f,       2.0f * sz,  -(fz + nz) * sz,
0.0f,       0.0f,       0.0f,       1.0f,
};

memcpy( mat->data, values, sizeof(values) );
}
*/