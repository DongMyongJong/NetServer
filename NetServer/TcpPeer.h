

/*
	TcpPeer
	- 하나의 TCP 접속을 나타내는 클래스

	* 이 클래스를 인스턴스화 하려면 자식 클래스를 만든 뒤
	  소켓 이벤트에 대한 각각의 가상 함수를 적절히 구현해야 한다.

	작성: oranze (oranze@wemade.com)
*/


#pragma once


#include "Monitor.h"
#include "SockAddr.h"


class TcpPeer : public Monitor
{
public:
	enum OPCODE { SEND, RECV };

	struct OVERLAPPEDEX : public OVERLAPPED
	{
		OPCODE	opcode;
		WSABUF	wsaBuf;		
		bool	inProgress;
	};

public:
	SOCKET			m_sock;
	SockAddr		m_remoteAddr;
	char			m_refCount;

	bool			m_sysClosing;	// 내부적으로 객체 종료를 처리중인 상태
									// - 시스템 함수 호출 실패 등

	bool			m_usrClosing;	// 사용자가 객체 종료를 요구한 상태
									// - CloseConnection을 호출했을 때

	OVERLAPPEDEX	m_olSend;
	int				m_sendBufMax;
	int				m_sendBufPos;
	char *			m_sendBuf;

	OVERLAPPEDEX	m_olRecv;
	int				m_recvBufMax;
	int				m_recvBufPos;
	char *			m_recvBuf;
	int				_device;

public:
	TcpPeer( int recvBufMax = 8192 );
	virtual ~TcpPeer();

	bool Init( int recvBufMax );
	void LinkAcceptedSocket( SOCKET sock );
	void Uninit();

	char * GetIP();
	int  GetPort();

	bool Send( char *p, int len );
	bool CloseConnection( bool graceful = true );

public:
	bool Recv();
	bool ProcessRecvBuffer();

public:
	/*
		OnInitComplete
		- 연결이 성립되었다.

		return false: 접속을 끊는다.
	*/
	virtual bool OnInitComplete() { return true; }

	/*
		OnSendComplete
		- 패킷이 송신되었다.
	*/
	virtual void OnSendComplete( char *p, int len ) {}

	/*
		OnExtractPacket
		- 패킷이 수신되었다. 완료 패킷이 있는지 검사가 필요하다.

		return:	완료 패킷의 길이 (발견되지 않으면 0)
	*/
	virtual int  OnExtractPacket( char *p, int len ) = 0;

	/*
		OnRecvComplete
		- 완료 패킷이 수신되었다.

		return false: 접속을 끊는다.
	*/
	virtual bool OnRecvComplete( char *p, int len ) = 0;

	/*
		OnError
		- 오버랩드 호출이 실패하였다. (errCode = WSAGetLastError)

		return false: 접속을 끊는다.
	*/
	virtual bool OnError( int errCode ) { return false; }
};
