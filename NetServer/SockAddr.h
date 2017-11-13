

/*
	SockAddr
	- 윈속 SOCKADDR 랩핑 클래스

	작성: oranze (oranze@wemade.com)
*/


#pragma once


#include <winsock2.h>


struct SockAddr : public SOCKADDR
{
public:
	SockAddr();
	SockAddr( int port );
	SockAddr( char *destAddr, int port );

	operator SockAddr * ();
	operator const SockAddr * ();
	operator SOCKADDR * ();
	operator const SOCKADDR * ();

	char *	GetIP();
	int		GetPort();
	
public:
	static bool IsIPAddress( char *addr );
};
