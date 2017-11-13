

/*
	TcpListener
	- TCP 접속 수락을 위한 클래스

	작성: oranze (oranze@wemade.com)
*/


#pragma once


#include "Monitor.h"
#include "SockAddr.h"


class TcpListener : public Monitor
{
public:
	SOCKET	m_sock;

public:
	TcpListener();
	virtual ~TcpListener();

	bool Init( SockAddr *addr );
	void Uninit();
};