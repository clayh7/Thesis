#pragma once

#include "Engine/Math/Euler.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Matrix4f.hpp"


//-------------------------------------------------------------------------------------------------
class Camera3D
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const float MOUSE_SENSITIVITY;
	static const float MOVE_SPEED;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	//#TODO: Change to Transform (add)
	Vector3f m_position;
	Euler m_orientation;

private:
	Matrix4f m_projection;
	float m_sensitivity;
	bool m_mouseControl;
	bool m_fitToWindow;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Camera3D( bool perspective );
	Camera3D( Vector3f const & position = Vector3f::ZERO, Euler const & orientation = Euler::ZERO, bool perspective = true );
	~Camera3D( );
	void Update( );
	void UpdateInput( );
	void FixAndClampAngles();

	void SetPosition( Vector3f const & position );
	void SetOrientation( Euler const & orientation );

	Euler GetOrientation() const { return m_orientation; }
	Vector3f GetForwardXYZ() const;
	Vector3f GetForwardXY() const;
	Vector3f GetLeftXY() const;
	Vector3f GetRightXY( ) const;
	Matrix4f GetViewMatrix( ) const;
	Matrix4f GetProjectionMatrix( ) const { return m_projection; }
};