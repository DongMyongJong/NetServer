

/*
	Monitor
	- ��32 ������ ����ȭ Ŭ����

	�ۼ�: oranze (oranze@wemade.com)
*/


#pragma once


#include <windows.h>


class Monitor
{
private:
	CRITICAL_SECTION m_cs;

public:
	Monitor();
	virtual ~Monitor();

	void Lock();
	void Unlock();
};
