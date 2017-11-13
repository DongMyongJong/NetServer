

/*
	TcpPeer
	- �ϳ��� TCP ������ ��Ÿ���� Ŭ����

	* �� Ŭ������ �ν��Ͻ�ȭ �Ϸ��� �ڽ� Ŭ������ ���� ��
	  ���� �̺�Ʈ�� ���� ������ ���� �Լ��� ������ �����ؾ� �Ѵ�.

	�ۼ�: oranze (oranze@wemade.com)
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

	bool			m_sysClosing;	// ���������� ��ü ���Ḧ ó������ ����
									// - �ý��� �Լ� ȣ�� ���� ��

	bool			m_usrClosing;	// ����ڰ� ��ü ���Ḧ �䱸�� ����
									// - CloseConnection�� ȣ������ ��

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
		- ������ �����Ǿ���.

		return false: ������ ���´�.
	*/
	virtual bool OnInitComplete() { return true; }

	/*
		OnSendComplete
		- ��Ŷ�� �۽ŵǾ���.
	*/
	virtual void OnSendComplete( char *p, int len ) {}

	/*
		OnExtractPacket
		- ��Ŷ�� ���ŵǾ���. �Ϸ� ��Ŷ�� �ִ��� �˻簡 �ʿ��ϴ�.

		return:	�Ϸ� ��Ŷ�� ���� (�߰ߵ��� ������ 0)
	*/
	virtual int  OnExtractPacket( char *p, int len ) = 0;

	/*
		OnRecvComplete
		- �Ϸ� ��Ŷ�� ���ŵǾ���.

		return false: ������ ���´�.
	*/
	virtual bool OnRecvComplete( char *p, int len ) = 0;

	/*
		OnError
		- �������� ȣ���� �����Ͽ���. (errCode = WSAGetLastError)

		return false: ������ ���´�.
	*/
	virtual bool OnError( int errCode ) { return false; }
};
