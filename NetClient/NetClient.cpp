// NetClient.cpp: implementation of the NetClient class.
//
//////////////////////////////////////////////////////////////////////

#include "NetClient.h"
#include "SockAddr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NetClient::NetClient()
{
	WSADATA wsaData;
	WSAStartup( 0x0202, &wsaData );
}

NetClient::~NetClient()
{
	WSACleanup();
}

bool NetClient::Connect(char *szSvrIpAddr, int nPort)
{
	SockAddr *destAddr = new SockAddr(szSvrIpAddr, nPort);
	
	_sock = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	if ( _sock == INVALID_SOCKET )
		return false;
	
	unsigned long nonblock = 1;
	if ( ioctlsocket( _sock, FIONBIO, &nonblock ) == SOCKET_ERROR )
		return false;
	
	if ( connect( _sock, destAddr, sizeof( SockAddr ) ) == SOCKET_ERROR )
	{
		if ( WSAGetLastError() != WSAEWOULDBLOCK )
			return false;
	}
	return true;
}
void NetClient::Disconnect()
{
	WSACleanup();
}
bool NetClient::Send(char *p, int len)
{
	if ( send( _sock, p, len, 0) == SOCKET_ERROR )
	{
		return false;
	}

	Sleep(100);
	return true;
}
