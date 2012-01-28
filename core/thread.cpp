#include "thread.h"

Thread::Thread()
	: m_threadHandle( NULL )
{
}


Thread::~Thread()
{
}


bool Thread::run()
{
	m_threadHandle = CreateThread(	NULL,	// Default security options
									0,		// Let platform determine stack size
									(LPTHREAD_START_ROUTINE)s_threadFunc,
									(void*)this,
									0,
									NULL );

	return m_threadHandle ? true : false;
}

void Thread::waitUntilComplete( DWORD timeoutMs )
{
	WaitForSingleObject( m_threadHandle, timeoutMs );
}


DWORD Thread::s_threadFunc( void* paramInstance)
{
	if( paramInstance )
	{
		return ((Thread*)paramInstance)->threadFunc();
	}

	return 0;
}
