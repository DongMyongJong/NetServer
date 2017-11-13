

#include "SockAddr.h"


SockAddr::SockAddr()
{
	memset( this, 0, sizeof( SOCKADDR ) );
}


SockAddr::SockAddr( int port )
{
	memset( this, 0, sizeof( SOCKADDR ) );

	SOCKADDR_IN *p = (SOCKADDR_IN *) this;

	p->sin_family		= AF_INET;
	p->sin_addr.s_addr	= htonl( INADDR_ANY );
	p->sin_port			= htons( port );
}


SockAddr::SockAddr( char *destAddr, int port )
{
	memset( this, 0, sizeof( SOCKADDR ) );
	
	SOCKADDR_IN *p = (SOCKADDR_IN *) this;

	p->sin_family	= AF_INET;
	p->sin_port		= htons( port );

	if ( IsIPAddress( destAddr ) )
	{
		p->sin_addr.s_addr = inet_addr( destAddr );
	}
	else
	{
		HOSTENT *he = gethostbyname( destAddr );
		if ( he )
			p->sin_addr.s_addr = *((unsigned long *) he->h_addr_list[0]);
	}
}


SockAddr::operator SockAddr * ()
{
	return this;
}


SockAddr::operator const SockAddr * ()
{
	return (const SockAddr *) this;
}


SockAddr::operator SOCKADDR * ()
{
	return (SOCKADDR *) this;
}


SockAddr::operator const SOCKADDR * ()
{
	return (const SOCKADDR *) this;
}


char * SockAddr::GetIP()
{
	return inet_ntoa( ((SOCKADDR_IN *) this)->sin_addr );
}


int SockAddr::GetPort()
{
	return ntohs( ((SOCKADDR_IN *) this)->sin_port );
}


bool SockAddr::IsIPAddress( char *addr )
{
	for ( ; *addr; addr++ )
	{
		if ( *addr == '.' )
			continue;

		if ( *addr < '0' || *addr > '9' )
			return false;
	}

	return true;
}