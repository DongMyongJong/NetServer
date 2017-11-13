

/*
	TcpServer_Connector
	- 접속 요청(connect)을 위해 TcpPeer를 관리하는 스레드

	작성: oranze (oranze@wemade.com)
*/


#pragma once


#include <list>
using namespace std;


#include "Thread.h"
#include "TcpPeer.h"


class TcpServer;
class TcpServer_Connector : public Thread, public Monitor
{	
protected:
	TcpServer *	m_parent;
	HANDLE		m_start;
	HANDLE		m_close;

	list< TcpPeer * >	m_listPeer;

public:
	TcpServer_Connector();
	virtual ~TcpServer_Connector();

	bool Init( TcpServer *parent );
	void Uninit();

	bool AddPeer( TcpPeer *obj );

protected:
	static unsigned __stdcall ThreadEntry( void *arg );
	void Connector();

	void AddNewPeer( HANDLE *events, int *eventCount );
	void ProcessWinsockEvent( HANDLE *events, int eventCount );
};