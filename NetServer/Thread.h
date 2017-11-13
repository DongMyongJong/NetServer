

/*
	Thread
	- ��32 ������ Ŭ����

	�ۼ�: oranze (oranze@wemade.com)
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
