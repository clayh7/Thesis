#include "Engine/Math/Transform.hpp"

#include "Engine/Math/Matrix4f.hpp"


//-------------------------------------------------------------------------------------------------
Transform::Transform( )
	: m_position( Vector3f::ZERO )
	, m_rotation( Matrix4f::IDENTITY )
	, m_scale( Vector3f::ONE )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Transform::Transform( Vector3f const & position )
	: Transform( position, Matrix4f::IDENTITY, Vector3f::ONE )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Transform::Transform( Vector3f const & position, Matrix4f const & rotation, Vector3f const & scale )
	: m_position( position )
	, m_rotation( rotation )
	, m_scale( scale )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Transform::~Transform( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Matrix4f Transform::GetModelMatrix( ) const
{
	Matrix4f scale = Matrix4f::IDENTITY;
	scale.SetScale( m_scale );
	Matrix4f model = Matrix4f::IDENTITY;
	model = m_rotation;// .GetRotationMatrix( );
	model.m_data[12] = m_position.x;
	model.m_data[13] = m_position.y;
	model.m_data[14] = m_position.z;
	return scale * model;
}

