

#include "TcpListener.h"


TcpListener::TcpListener()
{
	m_sock = INVALID_SOCKET;
}


TcpListener::~TcpListener()
{
	Uninit();
}


bool TcpListener::Init( SockAddr *addr )
{
	m_sock = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	if ( m_sock == INVALID_SOCKET )
		return false;

	if ( bind( m_sock, addr, sizeof( SockAddr ) ) == SOCKET_ERROR )
		return false;

	unsigned long nonblock = 1;
	if ( ioctlsocket( m_sock, FIONBIO, &nonblock ) == SOCKET_ERROR )
		return false;

	return true;
}


void TcpListener::Uninit()
{
	if ( m_sock != INVALID_SOCKET )
	{
		closesocket( m_sock );
		m_sock = INVALID_SOCKET;
	}
}