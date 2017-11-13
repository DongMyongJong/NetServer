

#include "Monitor.h"


Monitor::Monitor()
{
	InitializeCriticalSection( &m_cs );
}


Monitor::~Monitor()
{
	DeleteCriticalSection( &m_cs );
}


void Monitor::Lock()
{
	EnterCriticalSection( &m_cs );
}


void Monitor::Unlock()
{
	LeaveCriticalSection( &m_cs );
}
