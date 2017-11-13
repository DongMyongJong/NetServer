

#include "TcpPeer.h"
#include <stdlib.h>


TcpPeer::TcpPeer( int recvBufMax )
{
	Init( recvBufMax );
}


TcpPeer::~TcpPeer()
{
	Uninit();
}


bool TcpPeer::Init( int recvBufMax )
{
	m_sock			= INVALID_SOCKET;
	m_refCount		= 0;
	m_sysClosing	= false;
	m_usrClosing	= false;

	m_sendBufMax	= 0;
	m_sendBufPos	= 0;
	m_sendBuf		= NULL;
	memset( &m_olSend, 0, sizeof( OVERLAPPEDEX ) );
	m_olSend.opcode = OPCODE::SEND;

	m_recvBufMax	= recvBufMax;
	m_recvBufPos	= 0;
	m_recvBuf		= (char *) malloc( recvBufMax );
	if ( !m_recvBuf )
		return false;

	memset( &m_olRecv, 0, sizeof( OVERLAPPEDEX ) );
	m_olRecv.opcode	= OPCODE::RECV;
	memset( m_recvBuf, 0, recvBufMax );

	return true;
}


void TcpPeer::LinkAcceptedSocket( SOCKET sock )
{
	m_sock = sock;

	int addrLen = sizeof( SockAddr );
	getpeername( sock, &m_remoteAddr, &addrLen );
}


void TcpPeer::Uninit()
{
	if ( m_sendBuf )
		free( m_sendBuf );

	if ( m_recvBuf )
		free( m_recvBuf );

	if ( m_sock != INVALID_SOCKET )
		closesocket( m_sock );
}


char * TcpPeer::GetIP()
{
	return m_remoteAddr.GetIP();
}


int TcpPeer::GetPort()
{
	return m_remoteAddr.GetPort();
}


bool TcpPeer::Send( char *p, int len )
{
	if ( p )
	{
		int needLen = m_sendBufPos + len;
		
		if ( m_sendBufMax < needLen )
		{
			const int BLOCKSIZE = 256;
			int down = (needLen / BLOCKSIZE) * BLOCKSIZE;
			int rest = (needLen % BLOCKSIZE) > 0 ? BLOCKSIZE : 0;
			
			m_sendBufMax = down + rest;		
			m_sendBuf = (char *) realloc( m_sendBuf, m_sendBufMax );
			if ( !m_sendBufMax )
				return false;
		}
		
		memcpy( &m_sendBuf[ m_sendBufPos ], p, len );
		m_sendBufPos += len;
	}

	if ( m_usrClosing )
	{
		if ( !m_sendBufPos )
			shutdown( m_sock, SD_SEND );

		return true;
	}

	if ( m_sysClosing || m_olSend.inProgress )
		return true;

	memset( &m_olSend, 0, sizeof( OVERLAPPED ) );
	m_olSend.wsaBuf.buf = m_sendBuf;
	m_olSend.wsaBuf.len = m_sendBufPos;

	DWORD bytesSent;

	if ( WSASend( m_sock, &m_olSend.wsaBuf, 1, &bytesSent,
		0, (OVERLAPPED *) &m_olSend, NULL ) == SOCKET_ERROR )
	{
		if ( WSAGetLastError() != WSA_IO_PENDING )
			return OnError( WSAGetLastError() );
	}
	
	m_olSend.inProgress = true;
	m_refCount++;

	return true;
}


bool TcpPeer::CloseConnection( bool graceful )
{
	if ( m_sock == INVALID_SOCKET )
		return true;

	m_usrClosing = true;

	if ( graceful )
	{
		if ( !m_olSend.inProgress )
			shutdown( m_sock, SD_SEND );
	}
	else
		shutdown( m_sock, SD_SEND );

	return true;
}


bool TcpPeer::Recv()
{
	if ( m_sysClosing || m_olRecv.inProgress )
		return true;

	memset( &m_olRecv, 0, sizeof( OVERLAPPED ) );
	m_olRecv.wsaBuf.buf = m_recvBuf + m_recvBufPos;
	m_olRecv.wsaBuf.len = m_recvBufMax - m_recvBufPos;

	DWORD bytesReceived;
	DWORD flag = 0;

	if ( WSARecv( m_sock, &m_olRecv.wsaBuf, 1, &bytesReceived,
		&flag, (OVERLAPPED *) &m_olRecv, NULL ) == SOCKET_ERROR )
	{
		if ( WSAGetLastError() != WSA_IO_PENDING )
			return OnError( WSAGetLastError() );
	}

	m_olRecv.inProgress = true;
	m_refCount++;
	
	return true;
}


bool TcpPeer::ProcessRecvBuffer()
{
	int packetLen = OnExtractPacket( m_recvBuf, m_recvBufPos );
	if ( !packetLen )
	{
		if ( m_recvBufPos >= m_recvBufMax )
			return false;

		return true;
	}

	for (int i = packetLen; i <= strlen(m_recvBuf); i++)
	{
		m_recvBuf[i] = 0;
	}
	if ( !OnRecvComplete( m_recvBuf, packetLen ) )
		return false;

	m_recvBufPos -= packetLen;
	memmove( m_recvBuf, m_recvBuf + packetLen, m_recvBufPos );

	return true;
}
