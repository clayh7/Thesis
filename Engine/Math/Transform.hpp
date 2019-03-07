#pragma once

#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Matrix4f.hpp"


//-------------------------------------------------------------------------------------------------
class Transform
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Matrix4f m_rotation;
	Vector3f m_scale;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Transform( );
	Transform( Vector3f const & position );
	Transform( Vector3f const & position, Matrix4f const & rotation, Vector3f const & scale );
	~Transform( );

	Matrix4f GetModelMatrix( ) const;
};