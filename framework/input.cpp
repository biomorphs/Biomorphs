#include "framework\input.h"
#include "framework\graphics\D3D_app.h"

#define KEY_PRESSED(x) (x & 0x80) ? true : false
#define KEY_TOGGLED(x) (x & 0x01) ? true : false

InputModule::InputModule( void* userData )
{
	D3DAppConfig *config = (D3DAppConfig*)userData;
	if( config )
	{
		m_windowDimensions = Vector2(config->m_windowWidth, config->m_windowHeight);
	}
}

InputModule::~InputModule()
{
}

bool InputModule::startup()
{
	memset( m_win32keyState, 0, 256 * sizeof(unsigned char) );
	memset( m_keyState, 0, 256 * sizeof(KeyState) );

	return true;
}

bool InputModule::shutdown()
{
	return true;
}

bool InputModule::connect( IModuleConnector& connector )
{
	return true;
}

bool InputModule::update( Timer& timer )
{
	// Pass all previous state into keystates
	for( int k=0;k<256;++k )
	{
		m_keyState[k].m_lastKeyState = m_win32keyState[k];
	}

	// update win32 state
	if( !GetKeyboardState( (PBYTE)&m_win32keyState ) )
	{
		return true;
	}

	// mouse buttons
	SHORT LButtonDown = GetKeyState(VK_LBUTTON);
	SHORT RButtonDown = GetKeyState(VK_RBUTTON);
	SHORT MButtonDown = GetKeyState(VK_MBUTTON);
	m_mouseState.m_buttonState = (LButtonDown & 0x8000) ? MOUSE_LEFT : 0;
	m_mouseState.m_buttonState |= (RButtonDown & 0x8000) ? MOUSE_RIGHT : 0;
	m_mouseState.m_buttonState |= (MButtonDown & 0x8000) ? MOUSE_MIDDLE : 0;

	// mouse position
	POINT mousePos;
	GetCursorPos(&mousePos);
	m_mouseState.m_position = Vector2(mousePos.x, m_windowDimensions.y() - mousePos.y);

	for( int k=0;k<256;++k )
	{
		float newPressedTime=m_keyState[k].m_timePressed;
		if( KEY_PRESSED(m_win32keyState[k]) )
		{
			if( KEY_PRESSED(m_keyState[k].m_lastKeyState) )
			{	// button held down
				newPressedTime += timer.getDelta();
			}
			else
			{	// button pressed
				newPressedTime = 0.0f;
			}
		}
		else
		{
			if( KEY_PRESSED(m_keyState[k].m_lastKeyState ) )
			{	// button released
				newPressedTime = 0.0f;
			}
		}

		m_keyState[k].m_timePressed = newPressedTime;
	}

	return true;
}