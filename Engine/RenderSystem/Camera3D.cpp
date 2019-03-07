#include "Engine/RenderSystem/Camera3D.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/Math/Vector2i.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/Euler.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/UISystem/UISystem.hpp"


//-------------------------------------------------------------------------------------------------
STATIC const float Camera3D::MOUSE_SENSITIVITY = 0.05f;
STATIC const float Camera3D::MOVE_SPEED = 4.f;


//-------------------------------------------------------------------------------------------------
Camera3D::Camera3D( bool perspective )
	: Camera3D( Vector3f::ZERO, Euler::ZERO, perspective )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Camera3D::Camera3D( Vector3f const & position /*= Vector3::ZERO*/, Euler const & orientation /*= Euler::ZERO*/, bool perspective /*= true*/ )
	: m_position( position )
	, m_orientation( orientation )
	, m_projection( Matrix4f::IDENTITY )
	, m_sensitivity( MOUSE_SENSITIVITY )
	, m_mouseControl( false )
	, m_fitToWindow( true )
{
	if ( perspective )
	{
		float aspect = 16.f / 9.f;
		m_projection.MakePerspective( 60.f, aspect, 0.01f, 1000.f );
	}
	else
	{
		Vector2i windowDimensions = GetWindowDimensions( );
		m_projection.MakeOrthonormal( (float) windowDimensions.x, (float) windowDimensions.y, -100.f, 100.f );
	}
}


//-------------------------------------------------------------------------------------------------
Camera3D::~Camera3D()
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void Camera3D::Update( )
{
	if ( m_mouseControl )
	{
		UpdateInput( );
	}
	
	if( m_fitToWindow )
	{
		Vector2f uiDimensions = Vector2f( (float) UISystem::VIRTUAL_WIDTH, (float) UISystem::VIRTUAL_HEIGHT );
		m_projection.MakeOrthonormal( uiDimensions.x, uiDimensions.y, -100.f, 100.f );
		//m_projection.MakeOrthonormal( (float) 20.f, (float) 20.f, -100.f, 100.f );
		//m_projection.MakeOrthonormal( 1.f, 1.f, -1.f, 1.f );
		//m_projection = Matrix4f::IDENTITY;
		//#TODO: still more work to do to make this work with the UISystem
		//m_position = Vector2f( 0.f, 0.f );
		m_position = Vector2f( uiDimensions.x/2.f, uiDimensions.y/2.f );
	}
}


//-------------------------------------------------------------------------------------------------
void Camera3D::UpdateInput( )
{
	Vector2i mouseDelta = g_InputSystem->GetMouseHoldDeviation( );
	m_orientation.m_yawDegreesAboutY -= (float) mouseDelta.x * m_sensitivity;
	m_orientation.m_pitchDegreesAboutX += (float) mouseDelta.y * m_sensitivity;
	FixAndClampAngles( );

	float moveSpeed = MOVE_SPEED * Time::DELTA_SECONDS;

	if( g_InputSystem->IsKeyDown( Input::KEY_SHIFT ) )
	{
		moveSpeed *= 4.f;
	}

	if( g_InputSystem->IsKeyDown( 'W' ) )
	{
		m_position += GetForwardXY( ) * moveSpeed;
	}

	if( g_InputSystem->IsKeyDown( 'S' ) )
	{
		m_position -= GetForwardXY( ) * moveSpeed;
	}

	if( g_InputSystem->IsKeyDown( 'A' ) )
	{
		m_position += GetLeftXY( ) * moveSpeed;
	}

	if( g_InputSystem->IsKeyDown( 'D' ) )
	{
		m_position -= GetLeftXY( ) * moveSpeed;
	}

	if( g_InputSystem->IsKeyDown( 'E' ) )
	{
		m_position.y += moveSpeed;
	}

	if( g_InputSystem->IsKeyDown( 'Q' ) )
	{
		m_position.y -= moveSpeed;
	}
}


//-------------------------------------------------------------------------------------------------
void Camera3D::FixAndClampAngles()
{
	m_orientation.m_pitchDegreesAboutX = Clamp(m_orientation.m_pitchDegreesAboutX, -89.9f, 89.9f);
	m_orientation.m_yawDegreesAboutY = WrapDegrees0to360(m_orientation.m_yawDegreesAboutY);
	m_orientation.m_rollDegreesAboutZ = Clamp(m_orientation.m_rollDegreesAboutZ, -89.9f, 89.9f);
}


//-------------------------------------------------------------------------------------------------
void Camera3D::SetPosition( Vector3f const & position)
{
	m_position = position;
}


//-------------------------------------------------------------------------------------------------
void Camera3D::SetOrientation( Euler const & orientation)
{
	m_orientation = orientation;
}


//-------------------------------------------------------------------------------------------------
Vector3f Camera3D::GetForwardXYZ() const
{
	float cosYaw = CosDegrees( m_orientation.m_yawDegreesAboutY + 90.f );
	float sinYaw = SinDegrees( m_orientation.m_yawDegreesAboutY + 90.f );
	float cosPitch = CosDegrees( m_orientation.m_pitchDegreesAboutX );
	float sinPitch = SinDegrees( m_orientation.m_pitchDegreesAboutX );
	float cameraForwardX = cosYaw * cosPitch;
	float cameraForwardY = -sinPitch;
	float cameraForwardZ = -sinYaw * cosPitch;

	//#TODO: This is... correct I think. Maybe check?
	return Vector3f( cameraForwardX, cameraForwardY, cameraForwardZ );
}


//-------------------------------------------------------------------------------------------------
Vector3f Camera3D::GetForwardXY() const
{
	float cosYaw = CosDegrees( m_orientation.m_yawDegreesAboutY + 90.f );
	float sinYaw = SinDegrees( m_orientation.m_yawDegreesAboutY + 90.f );
	return Vector3f( cosYaw, 0.f, -sinYaw );
}


//-------------------------------------------------------------------------------------------------
Vector3f Camera3D::GetLeftXY( ) const
{
	Vector3f forwardXY = GetForwardXY( );
	return Vector3f( forwardXY.z, 0.f, -forwardXY.x );
}


//-------------------------------------------------------------------------------------------------
Vector3f Camera3D::GetRightXY( ) const
{
	Vector3f forwardXY = GetForwardXY( );
	return Vector3f( -forwardXY.z, 0.f, forwardXY.x );
}


//-------------------------------------------------------------------------------------------------
Matrix4f Camera3D::GetViewMatrix( ) const
{
	Matrix4f view( Matrix4f::IDENTITY );
	//view.MakeBasis( Vector3( 1.f, 0.f, 0.f ), Vector3( 0.f, 1.f, 0.f ), Vector3( 0.f, 0.f, -1.f ) );
	view.MakeView( m_position, m_orientation );
	view.InvertOrthonormal( );
	return view;
}