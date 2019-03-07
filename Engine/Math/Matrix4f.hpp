#pragma once

#include <vector>
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Math/Euler.hpp"


//-------------------------------------------------------------------------------------------------
class Matrix4f
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static Matrix4f const IDENTITY;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static Matrix4f GetEngineBasis( );
	static Matrix4f Orthonormal( float width, float height, float nz = -1.f, float fz = 1.f );
	static Matrix4f View( Vector3f const & position = Vector3f::ZERO, Euler const & orientation = Euler::ZERO );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	float m_data[16];

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Matrix4f( );
	Matrix4f
	(
		float r1c1, float r1c2, float r1c3, float r1c4,
		float r2c1, float r2c2, float r2c3, float r2c4,
		float r3c1, float r3c2, float r3c3, float r3c4,
		float r4c1, float r4c2, float r4c3, float r4c4
	);
	Matrix4f( float const * src );
	Matrix4f( Matrix4f const & src );
	Matrix4f( Vector4f const & row1, Vector4f const & row2, Vector4f const & row3, Vector4f const & row4 );
	Matrix4f( Euler const & eulerAngles );
	~Matrix4f( );

	void Matrix4f::operator=( float const * rhs );
	bool Matrix4f::operator==( Matrix4f const & rhs ) const;
	friend Matrix4f const operator*( Matrix4f const & lhs, Matrix4f const & rhs );
	friend Matrix4f const operator*( float lhs, Matrix4f const &rhs );
	friend Vector4f const operator*( Vector4f const & lhs, Matrix4f const & rhs );

	void Transpose( );
	void Orthogonalize( );
	void InvertOrthonormal( );

	void MakeOrthonormal( float width, float height, float nz, float fz );
	void MakePerspective( float fovDegreesY, float aspect, float nearZ, float farZ );
	void MakeLookAt( Vector3f const & position, Vector3f const & target, Vector3f const & upVector = Vector3f( 0.f, 0.f, 1.f ) );
	void MakeView( float yawDegrees, float pitchDegrees, float rollDegrees, Vector3f const & position );
	void MakeView( Vector3f const & position = Vector3f::ZERO, Euler const & orientation = Euler::ZERO );

	Vector4f const GetRow( int rowIndex ) const;
	Vector4f const GetCol( int colIndex ) const;
	void GetBasis( Vector3f * out_rightVector, Vector3f * out_upVector, Vector3f * out_forwardVector, Vector3f * out_translation ) const;
	Vector3f GetForward( ) const;
	Matrix4f GetInverseOrthonormal( ) const;
	Vector3f GetWorldPositionFromViewNoScale( ) const;

	void SetForward( Vector3f const & forward );
	void SetTranslation( Vector3f const & position );
	void SetTranslation( float setX, float setY, float setZ );
	void SetRotation( Euler const & orientation );
	void SetBasis( Vector3f const & rightVector, Vector3f const & upVector, Vector3f const & forwardVector );
	void SetScale( Vector3f const & scaleVector );
};