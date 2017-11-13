

/*
	Monitor
	- 윈32 스레드 동기화 클래스

	작성: oranze (oranze@wemade.com)
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
