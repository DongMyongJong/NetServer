

/*
	TcpServer_Acceptor
	- ���� ����(accept)�� ���� TcpListener�� �����ϴ� ������

	�ۼ�: oranze (oranze@wemade.com)
*/


#pragma once


#include <list>
using namespace std;


#include "Thread.h"
#include "TcpListener.h"


class TcpServer;
class TcpServer_Acceptor : public Thread, public Monitor
{	
protected:
	TcpServer *	m_parent;
	HANDLE		m_start;
	HANDLE		m_close;

	list< TcpListener * >	m_listListener;

public:
	TcpServer_Acceptor();
	virtual ~TcpServer_Acceptor();

	bool Init( TcpServer *parent );
	void Uninit();

	bool AddListener( TcpListener *obj );

protected:
	static unsigned __stdcall ThreadEntry( void *arg );
	void Acceptor();

	void ProcessWinsockEvent( HANDLE *events, int eventCount );
};
