#include "Thread.h"
#include <process.h>

Thread::Thread()
{
	m_id		= 0;
	m_handle	= NULL;
}


Thread::~Thread()
{
	Close();
}


DWORD Thread::GetID()
{
	return m_id;
}


HANDLE Thread::GetHandle()
{
	return m_handle;
}


bool Thread::Create( unsigned (__stdcall *func)( void * ), void *arg )
{
	m_handle = (HANDLE) _beginthreadex( NULL, 0, func, arg, 0, &m_id );
	if ( !m_handle )
		return false;

	return true;
}


void Thread::Close()
{
	if ( m_handle )
	{
		CloseHandle( m_handle );
		m_handle = NULL;
	}
	
	m_id = 0;
}


DWORD Thread::WaitForClosure( DWORD timeout )
{
	return WaitForSingleObject( m_handle, timeout );
}
