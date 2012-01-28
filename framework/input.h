#ifndef INPUT_MODULE_INCLUDED
#define INPUT_MODULE_INCLUDED

#include "core/module.h"
#include "core/vector2.h"

class InputModule : public Module
{
public:
	InputModule( void* userData );
	virtual ~InputModule();

	enum MouseButtons 
	{
		MOUSE_LEFT=1,
		MOUSE_RIGHT=2,
		MOUSE_MIDDLE=4,
		MOUSE_SCROLLUP=8,
		MOUSE_SCROLLDOWN=16
	};

	inline bool mouseButtonDown(MouseButtons button)
	{
		return (m_mouseState.m_buttonState & button) ? true : false;
	}

	inline Vector2 mousePosition()
	{
		return m_mouseState.m_position;
	}

	inline bool keyPressed( unsigned char key ) const
	{
		 return (m_win32keyState[key] & 0x80) ? true : false;
	}

	inline bool keyPressed( unsigned char key, float& timePressed ) const
	{
		timePressed = m_keyState[key].m_timePressed;
		 return (m_win32keyState[key] & 0x80) ? true : false;
	}

	inline void resetKeyPressTime( unsigned char key )
	{
		m_keyState[key].m_timePressed = 0.0f;
	}

	inline bool keyToggled( unsigned char key ) const
	{
		 return (m_win32keyState[key] & 0x01) ? true : false;
	}

protected:

	virtual bool startup();
	virtual bool connect( IModuleConnector& connector );
	virtual bool shutdown();
	virtual bool update( Timer& timer );

	struct KeyState
	{
		unsigned char m_lastKeyState;
		float m_timePressed;
	};

	struct MouseState
	{
		Vector2 m_position;
		unsigned char m_buttonState;
	};

private:
	unsigned char m_win32keyState[256];	// index=key,hsb=pressed, lsb=toggled
	KeyState m_keyState[256];
	MouseState m_mouseState;
	Vector2 m_windowDimensions;
};

#endif