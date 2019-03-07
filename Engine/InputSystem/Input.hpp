#pragma once

#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
class Input;
class XboxController;


//-------------------------------------------------------------------------------------------------
extern Input* g_InputSystem;


//-------------------------------------------------------------------------------------------------
class KeyState
{
public:
	bool m_isDown;
	int m_frameNumberLastChanged;
};


//-------------------------------------------------------------------------------------------------
class MouseState
{
public:
	bool m_isDown;
	int m_frameNumberLastChanged;
};


//-------------------------------------------------------------------------------------------------
enum eMouseButton
{
	eMouseButton_LEFT,
	eMouseButton_RIGHT,
	eMouseButton_COUNT,
};


//-------------------------------------------------------------------------------------------------
class Input
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const unsigned char KEY_ESCAPE		= 0x1B;
	static const unsigned char KEY_SHIFT		= 0x10;
	static const unsigned char KEY_CTRL			= 0x11;
	static const unsigned char KEY_UP			= 0x26;
	static const unsigned char KEY_DOWN			= 0x28;
	static const unsigned char KEY_LEFT			= 0x25;
	static const unsigned char KEY_RIGHT		= 0x27;
	static const unsigned char KEY_COLON		= 0xba;
	static const unsigned char KEY_TILDE		= 0xc0;
	static const unsigned char KEY_BACKSPACE	= 0x08;
	static const unsigned char KEY_ENTER		= 0x0d;
	static const unsigned char KEY_DASH			= 0xbd;
	static const unsigned char KEY_TAB			= 0x09;
	static const unsigned char KEY_PIPE			= 0xdc;
	static const unsigned char KEY_COMMA		= 0xbc;
	static const unsigned char KEY_PERIOD		= 0xbe;
	static const unsigned char KEY_NUMPAD0		= 0x60;
	static const unsigned char KEY_NUMPAD1		= 0x61;
	static const unsigned char KEY_NUMPAD2		= 0x62;
	static const unsigned char KEY_NUMPAD3		= 0x63;
	static const unsigned char KEY_NUMPAD4		= 0x64;
	static const unsigned char KEY_NUMPAD5		= 0x65;
	static const unsigned char KEY_NUMPAD6		= 0x66;
	static const unsigned char KEY_NUMPAD7		= 0x67;
	static const unsigned char KEY_NUMPAD8		= 0x68;
	static const unsigned char KEY_NUMPAD9		= 0x69;
	static const unsigned char KEY_F5			= 0x74;

	static const int NUM_KEYS = 256;
	static const int MAX_XBOX_CONTROLLERS = 4;
	static char const * CHAR_TYPED_EVENT;
	static char const * BACKSPACE_EVENT;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	int m_currentFrameNumber;
	
	//Mouse Data
	bool m_isMouseHidden;
	Vector2i m_mouseHoldDeviation;
	Vector2i m_mouseHoldPosition;
	bool m_isHoldPositionActive;
	
	MouseState m_mouseStates[eMouseButton_COUNT];

	bool m_isWheelMovingUp;
	int m_lastWheelChange;

	//Window Data
	bool m_isFocused;
	int m_lastFocusChange;

	//Keyboard Data
	KeyState m_keyStates[NUM_KEYS];

	//Xbox Controller Data
	XboxController* m_xboxControllers[MAX_XBOX_CONTROLLERS];

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Input( );
	~Input( );
	void Update( );
	void AdvanceFrameNumber( );
	void AddTypedCharacter( unsigned char asKey );

	void SetMouseHidden( bool hideMouse );
	Vector2i GetMousePosition( bool relativeToWindow = false ) const;
	Vector2i GetMouseHoldDeviation( ) const;
	void SetMousePosition( const Vector2i& setPosition );
	void SetMousePosition( int setX, int setY );
	void UpdateHoldPosition( );
	void HoldMousePosition( const Vector2i& setPosition );
	void HoldMousePosition( int setX, int setY );
	void ReleaseMouseHold( );
	
	void SetMouseStatus( const eMouseButton& button, bool isNowDown );
	bool WasMouseButtonJustPressed( const eMouseButton& button ) const;
	bool IsMouseButtonDown( const eMouseButton& button ) const;
	bool WasMouseButtonJustReleased( const eMouseButton& button ) const;
	
	void SetWheelStatus( int wheelDelta );
	bool JustScrolledUp( ) const;
	bool JustScrolledDown( ) const;
	
	void SetFocus( bool isFocus );
	bool GainedFocus( ) const;
	bool IsFocused( ) const;
	bool LostFocus( ) const;

	void SetKeyStatus( unsigned char asKey, bool isNowDown );
	bool WasKeyJustPressed( unsigned char checkKey ) const;
	bool IsKeyDown( unsigned char checkKey ) const;
	bool WasKeyJustReleased( unsigned char checkKey ) const;
	
	XboxController const & GetXboxController( const int& playerNum ) const { return *m_xboxControllers[playerNum]; }
};