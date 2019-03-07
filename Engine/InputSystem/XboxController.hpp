#pragma once

#include "Engine/Math/Vector2f.hpp"


//-------------------------------------------------------------------------------------------------
enum XboxHexCode
{
	XboxHexCode_Up = 1,
	XboxHexCode_Down = 2,
	XboxHexCode_Left = 4,
	XboxHexCode_Right = 8,
	XboxHexCode_Start = 16,
	XboxHexCode_Back = 32,
	XboxHexCode_LeftStickButton = 64,
	XboxHexCode_RightStickButton = 128,
	XboxHexCode_LeftBumper = 256,
	XboxHexCode_RightBumper = 512,
	XboxHexCode_A = 4096,
	XboxHexCode_B = 8192,
	XboxHexCode_X = 16384,
	XboxHexCode_Y = 32768,
};


//-------------------------------------------------------------------------------------------------
enum XboxButton
{
	XboxButton_Up,
	XboxButton_Down,
	XboxButton_Left,
	XboxButton_Right,
	XboxButton_Start,
	XboxButton_Back,
	XboxButton_LeftStickButton,
	XboxButton_RightStickButton,
	XboxButton_LeftBumper,
	XboxButton_RightBumper,
	XboxButton_A,
	XboxButton_B,
	XboxButton_X,
	XboxButton_Y,
	XboxButton_Count,
};


//-------------------------------------------------------------------------------------------------
enum XboxTrigger
{
	XboxTrigger_Left,
	XboxTrigger_Right,
};


//-------------------------------------------------------------------------------------------------
enum XboxStick
{
	XboxStick_Left,
	XboxStick_Right,
};


//-------------------------------------------------------------------------------------------------
enum XboxPlayer
{
	XboxPlayer_1,
	XboxPlayer_2,
	XboxPlayer_3,
	XboxPlayer_4,
};


//-------------------------------------------------------------------------------------------------
class ButtonState
{
public:
	bool m_isDown;
	int m_frameNumberLastChanged;
};


//-------------------------------------------------------------------------------------------------
class XboxController
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const float STICK_DEADZONE_MIN;
	static const float STICK_DEADZONE_MAX;

	static const float HEAVY_VIBRATE_AMOUNT;
	static const float LIGHT_VIBRATE_AMOUNT;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	int m_currentFrameNumber;
	ButtonState m_buttons[XboxButton_Count];
	float m_leftTrigger;
	float m_rightTrigger;
	Vector2f m_leftStick;
	float m_leftStickRadians;
	Vector2f m_rightStick;
	float m_rightStickRadians;
	bool m_isConnected;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	XboxController( );
	~XboxController( );
	void Update( int controllerIndex, int currentFrame );
	void SetButtonStatus( int buttonInput );
	void ParseInputSetTo( XboxHexCode const & hexToCheck, XboxButton const & setButton, int checkButton );
	void SetStickStatus( const float& leftStickXRaw, const float& leftStickYRaw, const float& rightStickXRaw, const float& rightStickYRaw );
	bool IsConnected() const;
	bool WasButtonJustPressed( XboxButton const & checkButton ) const;
	bool IsButtonDown( XboxButton const & checkButton ) const;
	bool WasButtonJustReleased( XboxButton const & checkButton ) const;
	float GetTrigger( XboxTrigger const & checkTrigger ) const;
	bool IsStickMoving( XboxStick const & checkStick ) const;
	Vector2f GetStickPosition( XboxStick const & checkStick ) const;
	float GetStickDirectionRadians( XboxStick const & checkStick ) const;
	static void Vibrate( int playerNum, int leftVal, int rightVal );
};