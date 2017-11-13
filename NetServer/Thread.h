

/*
	Thread
	- 윈32 스레드 클래스

	작성: oranze (oranze@wemade.com)
*/


#pragma once


#include <windows.h>


class Thread
{
protected:
	unsigned	m_id;
	HANDLE		m_handle;

public:
	Thread();
	virtual ~Thread();

	DWORD	GetID();
	HANDLE	GetHandle();

	bool	Create( unsigned (__stdcall *func)( void * ), void *arg );
	void	Close();
	DWORD	WaitForClosure( DWORD timeout );
};
