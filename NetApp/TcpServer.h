

/*
	TcpServer
	- TCP ������ ����� ���� ���� Ŭ����

	* �� Ŭ������ �ν��Ͻ�ȭ �Ϸ��� �ڽ� Ŭ������ ���� ��
	  ���� �̺�Ʈ�� ���� ������ ���� �Լ��� ������ �����ؾ� �Ѵ�.

	�ۼ�: oranze (oranze@wemade.com)
*/


#pragma once


#pragma warning( disable: 4786 )


#include "TcpServer_Dispatcher.h"
#include "TcpServer_Acceptor.h"
#include "TcpServer_Connector.h"
#include "TcpListener.h"
#include "TcpPeer.h"

/* Argument structure for SIO_KEEPALIVE_VALS */

struct tcp_keepalive {
    u_long  onoff;
    u_long  keepalivetime;
    u_long  keepaliveinterval;
};

// New WSAIoctl Options

#define SIO_RCVALL            _WSAIOW(IOC_VENDOR,1)
#define SIO_RCVALL_MCAST      _WSAIOW(IOC_VENDOR,2)
#define SIO_RCVALL_IGMPMCAST  _WSAIOW(IOC_VENDOR,3)
#define SIO_KEEPALIVE_VALS    _WSAIOW(IOC_VENDOR,4)
#define SIO_ABSORB_RTRALERT   _WSAIOW(IOC_VENDOR,5)
#define SIO_UCAST_IF          _WSAIOW(IOC_VENDOR,6)
#define SIO_LIMIT_BROADCASTS  _WSAIOW(IOC_VENDOR,7)
#define SIO_INDEX_BIND        _WSAIOW(IOC_VENDOR,8)
#define SIO_INDEX_MCASTIF     _WSAIOW(IOC_VENDOR,9)
#define SIO_INDEX_ADD_MCAST   _WSAIOW(IOC_VENDOR,10)
#define SIO_INDEX_DEL_MCAST   _WSAIOW(IOC_VENDOR,11)

class TcpServer
{
private:
	HANDLE		m_iocpHandle;
	int			m_numWorkers;
	Thread *	m_workerThreads;

	TcpServer_Dispatcher	m_dispatcher;
	TcpServer_Acceptor		m_acceptor;
	TcpServer_Connector		m_connector;

	friend TcpServer_Dispatcher;
	friend TcpServer_Acceptor;
	friend TcpServer_Connector;

public:	
	TcpServer();
	virtual ~TcpServer();

	bool Init( int dispatchCycle = 100, int numConcurrentThreads = 0, int numWorkers = 0 );
	void Uninit();

	bool Listen( TcpListener *obj );
	bool Connect( TcpPeer *obj, SockAddr *destAddr );

private:
	bool InitWorkerThread( int numWorkers );
	void UninitWorkerThread();

	void AssociatePeer( TcpPeer *obj );
	bool ClosePeer( TcpPeer *obj );

	static unsigned __stdcall ThreadEntry( void *arg );
	void WorkerThread();
	bool DispatchObject( TcpPeer *obj, int bytesTransferred, OVERLAPPED *ov );
	bool DispatchSend( TcpPeer *obj, int bytesTransferred );
	bool DispatchRecv( TcpPeer *obj, int bytesTransferred );

protected:
	/*
		OnAccept
		- accept �Ǿ��� ��

		return: 
		����� ������ ó�� �� TcpPeer�� �ν��Ͻ� ������
		- ���ϰ� �ν��Ͻ��� �����Ű�� ���� TcpPeer::LinkAcceptedSocket�� �ݵ�� ȣ���ؾ� �Ѵ�.

		return NULL: ������ ���´�.		
	*/
	virtual TcpPeer * OnAccept( TcpListener *listener, SOCKET sock ) { return NULL; }

	/*
		OnAcceptError
		- accept ȣ���� �������� ��
	*/
	virtual void OnAcceptError( TcpListener *listener, int errCode ) {}

	/*
		OnConnect
		- ������ �������� ��

		return false: IOCP�� ������ ������ ����Ѵ�.
	*/
	virtual bool OnConnect( TcpPeer *obj ) { return true; }

	/*
		OnConnectError
		- ������ �������� ��
	*/
	virtual void OnConnectError( TcpPeer *obj, int errCode ) {}

	/*
		OnClosePeer
		- TcpPeer�� ������ ����� �� (������ �������� ��)
	*/
	virtual void OnClosePeer( TcpPeer *obj ) {}

	/*
		OnBeginDispatch
		- TcpPeer���� ������ ��� ��Ŷ�� ó���ϱ� ����
	*/
	virtual void OnBeginDispatch() {}

	/*
		OnBeginDispatch
		- TcpPeer���� ������ ��� ��Ŷ�� ó���� ����
	*/
	virtual void OnEndDispatch() {}
};