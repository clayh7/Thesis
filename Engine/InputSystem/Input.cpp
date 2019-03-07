#include "Engine/InputSystem/Input.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/EventSystem/EventSystem.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
Input* g_InputSystem = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC char const * Input::CHAR_TYPED_EVENT = "CharacterTypedEvent";
STATIC char const * Input::BACKSPACE_EVENT = "BackspaceEvent";


//-------------------------------------------------------------------------------------------------
Input::Input( )
: m_currentFrameNumber( 0 )
, m_isMouseHidden( false )
, m_mouseHoldDeviation( Vector2i::ZERO )
, m_mouseHoldPosition( Vector2i::ZERO )
, m_isHoldPositionActive( false )
, m_isWheelMovingUp( false )
, m_lastWheelChange( -1 )
, m_isFocused( true )
, m_lastFocusChange( 0 )
{
	//Setup all controllers
	for ( int i = 0; i < MAX_XBOX_CONTROLLERS; i++ )
		m_xboxControllers[i] = new XboxController( );

	//Set all keys to "not down"
	for ( int keyIndex = 0; keyIndex < NUM_KEYS; keyIndex++ )
	{
		m_keyStates[keyIndex].m_isDown = false;
		m_keyStates[keyIndex].m_frameNumberLastChanged = -1;
	}
}


//-------------------------------------------------------------------------------------------------
Input::~Input( )
{
	for ( int i = 0; i < MAX_XBOX_CONTROLLERS; i++ )
	{
		delete m_xboxControllers[i];
		m_xboxControllers[i] = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
void Input::Update( )
{
	//AdvanceFrameNumber( );

	//Snap back to hold position every frame
	UpdateHoldPosition( );

	//Update all controllers
	for ( int i = 0; i < MAX_XBOX_CONTROLLERS; i++ )
		m_xboxControllers[i]->Update( i, m_currentFrameNumber );
}


//-------------------------------------------------------------------------------------------------
void Input::AdvanceFrameNumber( )
{
	m_currentFrameNumber++;
}


//-------------------------------------------------------------------------------------------------
void Input::AddTypedCharacter( unsigned char asKey )
{
	bool success = false;
	if( asKey >= 'a' && asKey <= 'z' )
	{
		success = true;
	}

	else if( asKey >= 'A' && asKey <= 'Z' )
	{
		success = true;
	}

	else if( asKey >= '0' && asKey <= '9' )
	{
		success = true;
	}

	else if( asKey == '-'
		|| asKey == '_'
		|| asKey == '+'
		|| asKey == '='
		|| asKey == '|'
		|| asKey == '/'
		|| asKey == '.'
		|| asKey == ':'
		|| asKey == ';'
		|| asKey == ' ' )
	{
		success = true;
	}

	if( success )
	{
		NamedProperties charTypedEvent;
		charTypedEvent.Set( "asKey", asKey );
		EventSystem::TriggerEvent( CHAR_TYPED_EVENT, charTypedEvent );
	}
}


//-------------------------------------------------------------------------------------------------
void Input::SetKeyStatus( unsigned char asKey, bool isNowDown )
{
	if( asKey == KEY_BACKSPACE && isNowDown )
	{
		EventSystem::TriggerEvent( BACKSPACE_EVENT );
	}

	m_keyStates[asKey].m_isDown = isNowDown;
	m_keyStates[asKey].m_frameNumberLastChanged = m_currentFrameNumber;
}


//-------------------------------------------------------------------------------------------------
bool Input::WasKeyJustPressed( unsigned char checkKey ) const
{
	const KeyState& ks = m_keyStates[checkKey];
	return (ks.m_isDown && (ks.m_frameNumberLastChanged == m_currentFrameNumber));
}

//-------------------------------------------------------------------------------------------------
bool Input::IsKeyDown( unsigned char checkKey ) const
{
	return m_keyStates[checkKey].m_isDown;
}


//-------------------------------------------------------------------------------------------------
bool Input::WasKeyJustReleased( unsigned char checkKey ) const
{
	const KeyState& ks = m_keyStates[checkKey];
	return (!ks.m_isDown && (ks.m_frameNumberLastChanged == m_currentFrameNumber));
}


//-------------------------------------------------------------------------------------------------
void Input::SetMouseStatus( const eMouseButton& button, bool isNowDown )
{
	m_keyStates[button].m_isDown = isNowDown;
	m_keyStates[button].m_frameNumberLastChanged = m_currentFrameNumber;
}


//-------------------------------------------------------------------------------------------------
bool Input::WasMouseButtonJustPressed( const eMouseButton& button ) const
{
	const KeyState& ks = m_keyStates[button];
	return (ks.m_isDown && (ks.m_frameNumberLastChanged == m_currentFrameNumber));
}


//-------------------------------------------------------------------------------------------------
bool Input::IsMouseButtonDown( const eMouseButton& button ) const
{
	return m_keyStates[button].m_isDown;
}


//-------------------------------------------------------------------------------------------------
bool Input::WasMouseButtonJustReleased( const eMouseButton& button ) const
{
	const KeyState& ks = m_keyStates[button];
	return (!ks.m_isDown && (ks.m_frameNumberLastChanged == m_currentFrameNumber));
}


//-------------------------------------------------------------------------------------------------
void Input::SetMouseHidden( bool hideMouse )
{
	if ( hideMouse )
		ShowCursor( FALSE );
	else
		ShowCursor( TRUE );

	m_isMouseHidden = hideMouse;
}


//-------------------------------------------------------------------------------------------------
Vector2i Input::GetMousePosition( bool relativeToWindow /*= false*/ ) const
{
	POINT cursorPos;
	GetCursorPos( &cursorPos );
	if( relativeToWindow )
	{
		ScreenToClient( g_EngineSystem->GetWindowHandle( ), &cursorPos );
	}

	//Invert Y because I want bottom to be 0 and top to be positive
	Vector2i windowDimensions = GetWindowDimensions( );
	return Vector2i( cursorPos.x, windowDimensions.y - cursorPos.y );
}


//-------------------------------------------------------------------------------------------------
Vector2i Input::GetMouseHoldDeviation() const
{
	return m_mouseHoldDeviation;
}


//-------------------------------------------------------------------------------------------------
void Input::SetMousePosition( const Vector2i& setPosition )
{
	SetMousePosition( setPosition.x, setPosition.y );
}


//-------------------------------------------------------------------------------------------------
void Input::SetMousePosition( int setX, int setY )
{
	SetCursorPos( setX, setY );
}


//-------------------------------------------------------------------------------------------------
void Input::UpdateHoldPosition( )
{
	if ( m_isHoldPositionActive )
	{
		Vector2i currentMousePosition = GetMousePosition( );
		m_mouseHoldDeviation = currentMousePosition - m_mouseHoldPosition;
		SetMousePosition( m_mouseHoldPosition );
	}
	else
	{
		m_mouseHoldDeviation = Vector2i::ZERO;
	}
}


//-------------------------------------------------------------------------------------------------
void Input::HoldMousePosition( const Vector2i& setPosition )
{
	HoldMousePosition( setPosition.x, setPosition.y );
}


//-------------------------------------------------------------------------------------------------
void Input::HoldMousePosition( int setX, int setY )
{
	SetMousePosition( setX, setY );
	m_mouseHoldPosition = Vector2i( setX, setY );
	m_mouseHoldDeviation = Vector2i::ZERO;
	m_isHoldPositionActive = true;
}


//-------------------------------------------------------------------------------------------------
void Input::ReleaseMouseHold( )
{
	m_mouseHoldPosition = Vector2i::ZERO;
	m_mouseHoldDeviation = Vector2i::ZERO;
	m_isHoldPositionActive = false;
}


//-------------------------------------------------------------------------------------------------
void Input::SetFocus( bool isFocus )
{
	m_isFocused = isFocus;
	m_lastFocusChange = m_currentFrameNumber;
}


//-------------------------------------------------------------------------------------------------
bool Input::GainedFocus() const
{
	return m_isFocused && (m_lastFocusChange == m_currentFrameNumber);
}


//-------------------------------------------------------------------------------------------------
bool Input::IsFocused() const
{
	return m_isFocused;
}


//-------------------------------------------------------------------------------------------------
bool Input::LostFocus() const
{
	return !m_isFocused && (m_lastFocusChange == m_currentFrameNumber);
}


//-------------------------------------------------------------------------------------------------
void Input::SetWheelStatus( int wheelDelta )
{
	m_isWheelMovingUp = wheelDelta > 0;
	m_lastWheelChange = m_currentFrameNumber;
}


//-------------------------------------------------------------------------------------------------
bool Input::JustScrolledUp( ) const
{
	return m_isWheelMovingUp && (m_lastWheelChange == m_currentFrameNumber);
}


//-------------------------------------------------------------------------------------------------
bool Input::JustScrolledDown( ) const
{
	return !m_isWheelMovingUp && (m_lastWheelChange == m_currentFrameNumber);
}