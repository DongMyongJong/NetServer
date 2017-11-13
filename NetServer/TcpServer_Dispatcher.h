

/*
	TcpServer_Dispatcher
	- ��� TcpPeer���� ������ ��Ŷ�� �ϰ������� ó���ϴ� ������

	�ۼ�: oranze (oranze@wemade.com)
*/


#pragma once


#include <vector>
using namespace std;


#include "Thread.h"
#include "TcpPeer.h"


class TcpServer;
class TcpServer_Dispatcher : public Thread, public Monitor
{
protected:
	TcpServer *	m_parent;
	HANDLE		m_close;
	int			m_dispatchCycle;

	vector< TcpPeer * >	m_listWait;
	vector< TcpPeer * >	m_listProcess;

public:
	TcpServer_Dispatcher();
	virtual ~TcpServer_Dispatcher();

	bool Init( TcpServer *parent, int m_dispatchCycle );
	void Uninit();

	bool AddPeer( TcpPeer *obj );

protected:
	static unsigned __stdcall ThreadEntry( void *arg );
	void Dispatcher();
};