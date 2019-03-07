#include "Engine/InputSystem/XboxController.hpp"

#include <Math.h>
#include <Windows.h>
#include <Xinput.h> // include the Xinput API
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Explicitly use XInput v9_1_0, since v1_4 is not supported under Windows 7
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
//XBOX stick reports values from 32767 to -32768
STATIC const float XboxController::STICK_DEADZONE_MIN = 8500.f;
STATIC const float XboxController::STICK_DEADZONE_MAX = 32000.f;

const float XboxController::HEAVY_VIBRATE_AMOUNT = 60000.f;
const float XboxController::LIGHT_VIBRATE_AMOUNT = 30000.f;


//-------------------------------------------------------------------------------------------------
XboxController::XboxController( )
: m_currentFrameNumber( 0 )
, m_isConnected( false )
, m_leftTrigger( 0.f )
, m_rightTrigger( 0.f )
, m_leftStick( 0.f, 0.f )
, m_rightStick( 0.f, 0.f )
{
	//Clear all buttons
	for ( int i = 0; i < XboxButton_Count; i++ )
	{
		m_buttons[i].m_frameNumberLastChanged = 0;
		m_buttons[i].m_isDown = false;
	}
}


//-------------------------------------------------------------------------------------------------
XboxController::~XboxController( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void XboxController::Update( int controllerIndex, int currentFrame )
{
	m_currentFrameNumber = currentFrame;
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof(xboxControllerState) );

	DWORD errorStatus = XInputGetState( controllerIndex, &xboxControllerState );
	if (errorStatus == ERROR_SUCCESS)
	{
		//If we even get in here then that means this controller is connected
		m_isConnected = true;

		//decode button input value and store which are pressed and when
		int buttonInputHex = xboxControllerState.Gamepad.wButtons;
		SetButtonStatus( buttonInputHex );

		//store float 0 - 1 inclusive for triggers
		m_leftTrigger = RangeMapNormalize( 0, 255, xboxControllerState.Gamepad.bLeftTrigger );
		m_rightTrigger = RangeMapNormalize( 0, 255, xboxControllerState.Gamepad.bRightTrigger );

		//Convert raw stick input into float 0 - 1 (inclusive)
		float leftStickXRaw = static_cast<float>(xboxControllerState.Gamepad.sThumbLX);
		float leftStickYRaw = static_cast<float>(xboxControllerState.Gamepad.sThumbLY);
		float rightStickXRaw = static_cast<float>(xboxControllerState.Gamepad.sThumbRX);
		float rightStickYRaw = static_cast<float>(xboxControllerState.Gamepad.sThumbRY);
		SetStickStatus( leftStickXRaw, leftStickYRaw, rightStickXRaw, rightStickYRaw );
	}
	else
	{
		m_isConnected = false;
	}
}


//-------------------------------------------------------------------------------------------------
void XboxController::SetButtonStatus( int buttonInput )
{
	ParseInputSetTo( XboxHexCode_Up, XboxButton_Up, buttonInput );
	ParseInputSetTo( XboxHexCode_Down, XboxButton_Down, buttonInput );
	ParseInputSetTo( XboxHexCode_Left, XboxButton_Left, buttonInput );
	ParseInputSetTo( XboxHexCode_Right, XboxButton_Right, buttonInput );
	ParseInputSetTo( XboxHexCode_Start, XboxButton_Start, buttonInput );
	ParseInputSetTo( XboxHexCode_Back, XboxButton_Back, buttonInput );
	ParseInputSetTo( XboxHexCode_LeftStickButton, XboxButton_LeftStickButton, buttonInput );
	ParseInputSetTo( XboxHexCode_RightStickButton, XboxButton_RightStickButton, buttonInput );
	ParseInputSetTo( XboxHexCode_LeftBumper, XboxButton_LeftBumper, buttonInput );
	ParseInputSetTo( XboxHexCode_RightBumper, XboxButton_RightBumper, buttonInput );
	ParseInputSetTo( XboxHexCode_A, XboxButton_A, buttonInput );
	ParseInputSetTo( XboxHexCode_B, XboxButton_B, buttonInput );
	ParseInputSetTo( XboxHexCode_X, XboxButton_X, buttonInput );
	ParseInputSetTo( XboxHexCode_Y, XboxButton_Y, buttonInput );
}


//-------------------------------------------------------------------------------------------------
void XboxController::ParseInputSetTo( XboxHexCode const & hexToCheck, XboxButton const & setButton, int checkButton )
{
	if ((checkButton & hexToCheck) == hexToCheck)
	{
		if (!m_buttons[setButton].m_isDown)
		{
			m_buttons[setButton].m_isDown = true;
			m_buttons[setButton].m_frameNumberLastChanged = m_currentFrameNumber;
		}
	}
	else
	{
		if (m_buttons[setButton].m_isDown)
		{
			m_buttons[setButton].m_isDown = false;
			m_buttons[setButton].m_frameNumberLastChanged = m_currentFrameNumber;
		}
	}
}


//-------------------------------------------------------------------------------------------------
// #TODO: Change everything in class to Degrees, and remove Math.h
void XboxController::SetStickStatus( const float& leftStickXRaw, const float& leftStickYRaw, const float& rightStickXRaw, const float& rightStickYRaw )
{
	//Left Stick
	float thetaRadians_leftStick = atan2( leftStickYRaw, leftStickXRaw );
	float radius_leftStick = sqrt( leftStickYRaw * leftStickYRaw + leftStickXRaw * leftStickXRaw );
	float normalizedRadius = 0.f;

	if (radius_leftStick < STICK_DEADZONE_MIN)
		normalizedRadius = 0.f;
	else if (radius_leftStick > STICK_DEADZONE_MAX)
		normalizedRadius = 1.f;
	else
		normalizedRadius = RangeMapNormalize( STICK_DEADZONE_MIN, STICK_DEADZONE_MAX, radius_leftStick );
	float leftStickX = normalizedRadius * cos( thetaRadians_leftStick );
	float leftStickY = normalizedRadius * sin( thetaRadians_leftStick );
	m_leftStick = Vector2f( leftStickX, leftStickY );
	m_leftStickRadians = thetaRadians_leftStick;

	//Right Stick
	float thetaRadians_rightStick = atan2( rightStickYRaw, rightStickXRaw );
	float radius_rightStick = sqrt( rightStickYRaw * rightStickYRaw + rightStickXRaw * rightStickXRaw );

	if (radius_rightStick < STICK_DEADZONE_MIN)
		normalizedRadius = 0.f;
	else if (radius_rightStick > STICK_DEADZONE_MAX)
		normalizedRadius = 1.f;
	else
		normalizedRadius = RangeMapNormalize( STICK_DEADZONE_MIN, STICK_DEADZONE_MAX, radius_rightStick );
	float rightStickX = normalizedRadius * cos( thetaRadians_rightStick );
	float rightStickY = normalizedRadius * sin( thetaRadians_rightStick );
	m_rightStick = Vector2f( rightStickX, rightStickY );
	m_rightStickRadians = thetaRadians_rightStick;
}


//-------------------------------------------------------------------------------------------------
bool XboxController::IsConnected() const
{
	return m_isConnected;
}


//-------------------------------------------------------------------------------------------------
bool XboxController::WasButtonJustPressed( XboxButton const & checkButton ) const
{
	const ButtonState& bs = m_buttons[checkButton];
	return (bs.m_isDown && (bs.m_frameNumberLastChanged == m_currentFrameNumber));
}


//-------------------------------------------------------------------------------------------------
bool XboxController::IsButtonDown( XboxButton const & checkButton ) const
{
	return m_buttons[checkButton].m_isDown;
}


//-------------------------------------------------------------------------------------------------
bool XboxController::WasButtonJustReleased( XboxButton const & checkButton ) const
{
	const ButtonState& bs = m_buttons[checkButton];
	return (!bs.m_isDown && (bs.m_frameNumberLastChanged == m_currentFrameNumber));
}


//-------------------------------------------------------------------------------------------------
float XboxController::GetTrigger( XboxTrigger const & checkTrigger ) const
{
	if (checkTrigger == XboxTrigger_Left)
		return m_leftTrigger;
	else if ( checkTrigger == XboxTrigger_Right )
		return m_rightTrigger;
	else
		return 0.f;
}


//-------------------------------------------------------------------------------------------------
bool XboxController::IsStickMoving( XboxStick const & checkStick ) const
{
	if ( checkStick == XboxStick_Left )
	{
		if ( m_leftStick.x != 0.f && m_leftStick.y != 0.f )
			return true;
		else
			return false;
	}
	else
	{
		if ( m_rightStick.x != 0.f && m_rightStick.y != 0.f )
			return true;
		else
			return false;
	}
}


//-------------------------------------------------------------------------------------------------
Vector2f XboxController::GetStickPosition( XboxStick const & checkStick ) const
{
	if ( checkStick == XboxStick_Left )
		return m_leftStick;
	else
		return m_rightStick;
}


//-------------------------------------------------------------------------------------------------
float XboxController::GetStickDirectionRadians( XboxStick const & checkStick ) const
{
	if ( checkStick == XboxStick_Left )
		return m_leftStickRadians;
	else
		return m_rightStickRadians;
}


//-------------------------------------------------------------------------------------------------
void XboxController::Vibrate( int playerNum, int leftVal, int rightVal )
{
	// Create a Vibraton State
	XINPUT_VIBRATION Vibration;

	// Set the Vibration Values
	Vibration.wLeftMotorSpeed = (WORD)leftVal;
	Vibration.wRightMotorSpeed = (WORD)rightVal;

	// Vibrate the controller
	XInputSetState( (DWORD)playerNum, &Vibration );
}