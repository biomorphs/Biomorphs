#ifndef THREAD_INCLUDED
#define THREAD_INCLUDED

#include <Windows.h>

class Thread
{
public:
	Thread();
	~Thread();

	bool run( );
	void waitUntilComplete( DWORD timeoutMs=INFINITE );
protected:
	HANDLE m_threadHandle;

	virtual bool threadFunc() = 0;

private:
	static DWORD s_threadFunc( void* paramInstance );
};

#endif