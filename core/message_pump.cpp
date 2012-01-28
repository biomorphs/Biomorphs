#include "core/message_pump.h"
#include <Windows.h>

MessagePump::MessagePump()
	: m_quitRequested( false )
{
}

bool MessagePump::update()
{
	MSG msg = {0};
	if( !m_quitRequested )
	{
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
		if( msg.message == WM_QUIT )
		{
			m_quitRequested = true;
		}
	}

	return !m_quitRequested;
}