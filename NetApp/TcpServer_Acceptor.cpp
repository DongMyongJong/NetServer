

#include "TcpServer_Acceptor.h"
#include "TcpServer.h"


TcpServer_Acceptor::TcpServer_Acceptor()
{
	m_parent	= NULL;
	m_start		= NULL;
	m_close		= NULL;
}


TcpServer_Acceptor::~TcpServer_Acceptor()
{
	Uninit();
}


bool TcpServer_Acceptor::Init( TcpServer *parent )
{
	m_parent = parent;

	m_start = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_close = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( !m_start || !m_close )
		return false;

	if ( !Create( ThreadEntry, this ) )
		return false;

	return true;
}


void TcpServer_Acceptor::Uninit()
{	
	if ( GetHandle() )
	{
		SetEvent( m_close );
		WaitForClosure( INFINITE );
		Close();

		CloseHandle( m_close );
		CloseHandle( m_start );
	}

	m_listListener.clear();
}


bool TcpServer_Acceptor::AddListener( TcpListener *obj )
{
	Lock();
	m_listListener.push_back( obj );
	Unlock();
	
	SetEvent( m_start );
	
	return true;
}


unsigned TcpServer_Acceptor::ThreadEntry( void *arg )
{
	((TcpServer_Acceptor *) arg)->Acceptor();

	return 0;
}


void TcpServer_Acceptor::Acceptor()
{
	HANDLE	events[MAXIMUM_WAIT_OBJECTS] = { m_start, m_close, (HANDLE) -1, };
	int		eventCount = 2;
	int		ret;

	for ( int i = 2; i < MAXIMUM_WAIT_OBJECTS; i++ )
		events[i] = CreateEvent( NULL, FALSE, FALSE, NULL );

	while ( true )
	{
		ret = WSAWaitForMultipleEvents( eventCount, events, FALSE, INFINITE, true );
		if ( ret == WAIT_OBJECT_0 )
		{
			Lock();

			list< TcpListener * >::iterator iter = m_listListener.begin();

			eventCount = 2;
			for ( ; iter != m_listListener.end(); iter++ )
			{
				WSAResetEvent( events[ eventCount ] );
				WSAEventSelect( (*iter)->m_sock, events[ eventCount ], FD_ACCEPT );

				eventCount++;
			}

			Unlock();
			continue;
		}
		else if ( ret == WAIT_OBJECT_0 + 1 )
			break;

		ProcessWinsockEvent( events, eventCount );
	}

	for ( int i = 2; i < MAXIMUM_WAIT_OBJECTS; i++ )
		CloseHandle( events[i] );
}

void TcpServer_Acceptor::ProcessWinsockEvent( HANDLE *events, int eventCount )
{
	WSANETWORKEVENTS	eventResult;
	
	SOCKET		sock;
	SockAddr	addr;
	int			addrLen;

	Lock();

	list< TcpListener * >::iterator iter = m_listListener.begin();
	TcpListener	*listener;
	TcpPeer		*obj;

	for ( int i = 2; i < eventCount; i++, iter++ )
	{
		listener = *iter;

		WSAEnumNetworkEvents( listener->m_sock, events[i], &eventResult );
		if ( !eventResult.lNetworkEvents )
			continue;

		addrLen = sizeof( addr );
		sock = accept( listener->m_sock, &addr, &addrLen );
		if ( sock == INVALID_SOCKET )
		{
			m_parent->OnAcceptError( listener, WSAGetLastError() );
			continue;
		}

		obj = m_parent->OnAccept( listener, sock );
		if ( !obj )
		{
			closesocket( sock );
			continue;
		}

		m_parent->AssociatePeer( obj );
	}

	Unlock();
}