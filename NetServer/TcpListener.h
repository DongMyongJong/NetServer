

/*
	TcpListener
	- TCP ���� ������ ���� Ŭ����

	�ۼ�: oranze (oranze@wemade.com)
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