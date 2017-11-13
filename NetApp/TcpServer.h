

/*
	TcpServer
	- TCP 서버를 만들기 위한 메인 클래스

	* 이 클래스를 인스턴스화 하려면 자식 클래스를 만든 뒤
	  소켓 이벤트에 대한 각각의 가상 함수를 적절히 구현해야 한다.

	작성: oranze (oranze@wemade.com)
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
		- accept 되었을 때

		return: 
		연결된 소켓을 처리 할 TcpPeer의 인스턴스 포인터
		- 소켓과 인스턴스를 연결시키기 위해 TcpPeer::LinkAcceptedSocket를 반드시 호출해야 한다.

		return NULL: 접속을 끊는다.		
	*/
	virtual TcpPeer * OnAccept( TcpListener *listener, SOCKET sock ) { return NULL; }

	/*
		OnAcceptError
		- accept 호출이 실패했을 때
	*/
	virtual void OnAcceptError( TcpListener *listener, int errCode ) {}

	/*
		OnConnect
		- 접속이 성공했을 때

		return false: IOCP와 소켓의 연결을 취소한다.
	*/
	virtual bool OnConnect( TcpPeer *obj ) { return true; }

	/*
		OnConnectError
		- 접속이 실패했을 때
	*/
	virtual void OnConnectError( TcpPeer *obj, int errCode ) {}

	/*
		OnClosePeer
		- TcpPeer가 완전히 종료될 때 (접속이 끊어졌을 때)
	*/
	virtual void OnClosePeer( TcpPeer *obj ) {}

	/*
		OnBeginDispatch
		- TcpPeer들이 수신한 모든 패킷을 처리하기 직전
	*/
	virtual void OnBeginDispatch() {}

	/*
		OnBeginDispatch
		- TcpPeer들이 수신한 모든 패킷을 처리한 직후
	*/
	virtual void OnEndDispatch() {}
};