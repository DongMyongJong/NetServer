

#include "TcpServer.h"
//#include <mstcpip.h>


TcpServer::TcpServer()
{
	WSADATA wsaData;
	WSAStartup( 0x0202, &wsaData );

	m_iocpHandle	= NULL;
	m_numWorkers	= 0;
	m_workerThreads	= NULL;
}


TcpServer::~TcpServer()
{
	Uninit();

	WSACleanup();
}


bool TcpServer::Init( int dispatchCycle, int numConcurrentThreads, int numWorkers )
{
	m_iocpHandle = CreateIoCompletionPort( 
		INVALID_HANDLE_VALUE, NULL, 0, numConcurrentThreads );
	if ( !m_iocpHandle )
		return false;

	if ( !InitWorkerThread( numWorkers ) )
		return false;

	if ( !m_dispatcher.Init( this, dispatchCycle ) )
		return false;

	if ( !m_acceptor.Init( this ) )
		return false;

	if ( !m_connector.Init( this ) )
		return false;

	return true;
}


void TcpServer::Uninit()
{
	UninitWorkerThread();
	
	m_dispatcher.Uninit();
	m_acceptor.Uninit();
	m_connector.Uninit();

	if ( m_iocpHandle )
	{
		CloseHandle( m_iocpHandle );
		m_iocpHandle = NULL;
	}
}


bool TcpServer::Listen( TcpListener *obj )
{
	if ( listen( obj->m_sock, SOMAXCONN ) == SOCKET_ERROR )
		return false;

	if ( !m_acceptor.AddListener( obj ) )
		return false;

	return true;
}


bool TcpServer::Connect( TcpPeer *obj, SockAddr *destAddr )
{
	obj->m_sock = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
	if ( obj->m_sock == INVALID_SOCKET )
		return false;

	unsigned long nonblock = 1;
	if ( ioctlsocket( obj->m_sock, FIONBIO, &nonblock ) == SOCKET_ERROR )
		return false;

	if ( connect( obj->m_sock, destAddr, sizeof( SockAddr ) ) == SOCKET_ERROR )
	{
		if ( WSAGetLastError() != WSAEWOULDBLOCK )
			return false;
	}

	if ( !m_connector.AddPeer( obj ) )
		return false;

	return true;
}


bool TcpServer::InitWorkerThread( int numWorkers )
{
	SYSTEM_INFO si;
	GetSystemInfo( &si );

	if ( !numWorkers )
		numWorkers = si.dwNumberOfProcessors * 2;

	m_numWorkers = numWorkers;
	m_workerThreads = new Thread[ numWorkers ];
	if ( !m_workerThreads )
		return false;

	for ( int i = 0; i < numWorkers; i++ )
	{
		if ( !m_workerThreads[i].Create( ThreadEntry, this ) )
			return false;
	}

	return true;
}


void TcpServer::UninitWorkerThread()
{
	if ( m_workerThreads )
	{
		for ( int i = 0; i < m_numWorkers; i++ )
			PostQueuedCompletionStatus( m_iocpHandle, 0, 0, NULL );

		for ( int i = 0; i < m_numWorkers; i++ )
			m_workerThreads[i].WaitForClosure( INFINITE );

		delete[] m_workerThreads;
		m_workerThreads	= NULL;
	}

	m_numWorkers = 0;
}


void TcpServer::AssociatePeer( TcpPeer *obj )
{
	tcp_keepalive keepAlive = { TRUE, 10000, 1000 };

	obj->Lock();

	DWORD tmp;
	WSAIoctl( obj->m_sock, SIO_KEEPALIVE_VALS, &keepAlive, sizeof( keepAlive ),
		0, 0, &tmp, NULL, NULL );

	CreateIoCompletionPort( (HANDLE) obj->m_sock, m_iocpHandle, (DWORD) obj, NULL );

	if ( !obj->Recv() || !obj->OnInitComplete() )
	{
		if ( ClosePeer( obj ) )
			return;
	}

	obj->Unlock();
}


bool TcpServer::ClosePeer( TcpPeer *obj )
{
	obj->m_sysClosing = true;

	if ( obj->m_refCount )
		return false;

	obj->Unlock();

	OnClosePeer( obj );

	return true;
}


unsigned TcpServer::ThreadEntry( void *arg )
{
	((TcpServer *) arg)->WorkerThread();

	return 0;
}


void TcpServer::WorkerThread()
{
	BOOL			ret;
	DWORD			bytesTransferred;
	TcpPeer *		obj;
	OVERLAPPED *	overlapped;

	while ( true )
	{
		ret = GetQueuedCompletionStatus( 
			m_iocpHandle,
			&bytesTransferred,
			(DWORD *) &obj,
			(OVERLAPPED **) &overlapped,
			INFINITE );

		if ( !obj || !overlapped )
			break;

		obj->Lock();
		obj->m_refCount--;

		if ( 
			!ret				||
			!bytesTransferred	||
			obj->m_sysClosing	||
			!DispatchObject( obj, bytesTransferred, overlapped ) )
		{
			if ( ClosePeer( obj ) )
				continue;
		}

		obj->Unlock();
	}
}


bool TcpServer::DispatchObject( TcpPeer *obj, int bytesTransferred, OVERLAPPED *ov )
{
	TcpPeer::OVERLAPPEDEX *ovex = (TcpPeer::OVERLAPPEDEX *) ov;

	switch ( ovex->opcode )
	{
	case TcpPeer::SEND:
		DispatchSend( obj, bytesTransferred );
		break;

	case TcpPeer::RECV:
		DispatchRecv( obj, bytesTransferred );
		break;
	}

	return true;
}


bool TcpServer::DispatchSend( TcpPeer *obj, int bytesTransferred )
{
	obj->m_olSend.inProgress = false;

	obj->OnSendComplete( obj->m_olSend.wsaBuf.buf, obj->m_olSend.wsaBuf.len );

	obj->m_sendBufPos -= bytesTransferred;
	memmove( obj->m_sendBuf, &obj->m_sendBuf[ bytesTransferred ], obj->m_sendBufPos );	

	if ( obj->m_sendBufPos || obj->m_usrClosing )
		obj->Send( NULL, 0 );

	return true;
}


bool TcpServer::DispatchRecv( TcpPeer *obj, int bytesTransferred )
{
	obj->m_olRecv.inProgress = false;
	obj->m_recvBufPos += bytesTransferred;

	m_dispatcher.AddPeer( obj );

	return true;
}
