

#include "TcpServer_Connector.h"
#include "TcpServer.h"


TcpServer_Connector::TcpServer_Connector()
{	
	m_parent	= NULL;
	m_start		= NULL;
	m_close		= NULL;
}


TcpServer_Connector::~TcpServer_Connector()
{
	Uninit();
}


bool TcpServer_Connector::Init( TcpServer *parent )
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


void TcpServer_Connector::Uninit()
{
	if ( GetHandle() )
	{
		SetEvent( m_close );
		WaitForClosure( INFINITE );
		Close();

		CloseHandle( m_close );
		CloseHandle( m_start );
	}

	m_listPeer.clear();
}


bool TcpServer_Connector::AddPeer( TcpPeer *obj )
{
	Lock();
	m_listPeer.push_back( obj );
	Unlock();
	
	SetEvent( m_start );

	return true;
}


unsigned TcpServer_Connector::ThreadEntry( void *arg )
{
	((TcpServer_Connector *) arg)->Connector();	

	return 0;
}


void TcpServer_Connector::Connector()
{	
	HANDLE	events[MAXIMUM_WAIT_OBJECTS] = { m_start, m_close, (HANDLE) -1, };
	int		eventCount = 2;
	int		ret;

	for ( int i = 2; i < MAXIMUM_WAIT_OBJECTS; i++ )
		events[i] = CreateEvent( NULL, FALSE, FALSE, NULL );

	while ( true )
	{
		ret = WSAWaitForMultipleEvents( eventCount, events, FALSE, INFINITE, FALSE );
		if ( ret == WAIT_OBJECT_0 )
		{
			AddNewPeer( events, &eventCount );
			continue;
		}
		else if ( ret == WAIT_OBJECT_0 + 1 )
			break;

		ProcessWinsockEvent( events, eventCount );
	}

	for ( int i = 2; i < MAXIMUM_WAIT_OBJECTS; i++ )
		CloseHandle( events[i] );
}


void TcpServer_Connector::AddNewPeer( HANDLE *events, int *eventCount )
{
	Lock();
	
	list< TcpPeer * >::iterator iter = m_listPeer.begin();
	*eventCount = 2;

	for ( ; iter != m_listPeer.end(); iter++ )
	{
		WSAResetEvent( events[ *eventCount ] );
		WSAEventSelect( (*iter)->m_sock, events[ *eventCount ], FD_CONNECT );
		
		(*eventCount)++;
	}

	Unlock();
}


void TcpServer_Connector::ProcessWinsockEvent( HANDLE *events, int eventCount )
{
	WSANETWORKEVENTS eventResult;
	TcpPeer	*obj;

	Lock();

	for ( int i = 2; i < eventCount; i++ )
	{
		obj = m_listPeer.front();
		m_listPeer.pop_front();

		WSAEnumNetworkEvents( obj->m_sock, events[i], &eventResult );
		if ( !eventResult.lNetworkEvents )
			continue;

		if ( eventResult.iErrorCode[ FD_CONNECT_BIT ] )
		{
			m_parent->OnConnectError( obj, eventResult.iErrorCode[ FD_CONNECT_BIT ] );
			continue;
		}

		if ( !m_parent->OnConnect( obj ) )
			continue;

		m_parent->AssociatePeer( obj );
	}

	Unlock();
}