

#include "TcpServer_Dispatcher.h"
#include "TcpServer.h"


TcpServer_Dispatcher::TcpServer_Dispatcher()
{
	m_parent		= NULL;
	m_close			= NULL;
	m_dispatchCycle	= 0;
}


TcpServer_Dispatcher::~TcpServer_Dispatcher()
{
	Uninit();
}


bool TcpServer_Dispatcher::Init( TcpServer *parent, int dispatchCycle )
{
	m_parent		= parent;
	m_dispatchCycle	= dispatchCycle;

	m_close = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( !m_close )
		return false;

	if ( !Create( ThreadEntry, this ) )
		return false;

	m_listWait.reserve( 1000 );
	m_listProcess.reserve( 1000 );

	return true;
}


void TcpServer_Dispatcher::Uninit()
{
	if ( GetHandle() )
	{
		SetEvent( m_close );
		WaitForClosure( INFINITE );
		Close();

		CloseHandle( m_close );
	}

	m_listWait.clear();
	m_listProcess.clear();
}


bool TcpServer_Dispatcher::AddPeer( TcpPeer *obj )
{
	Lock();
	m_listWait.push_back( obj );
	Unlock();
	
	obj->m_refCount++;

	return true;
}


unsigned TcpServer_Dispatcher::ThreadEntry( void *arg )
{
	((TcpServer_Dispatcher *) arg)->Dispatcher();

	return 0;
}


void TcpServer_Dispatcher::Dispatcher()
{
	vector< TcpPeer * >::iterator iter;
	TcpPeer *obj;

	while ( true )
	{
		if ( WaitForSingleObject( m_close, m_dispatchCycle ) == WAIT_OBJECT_0 )
			break;

		Lock();
		m_listWait.swap( m_listProcess );
		Unlock();

		m_parent->OnBeginDispatch();

		for ( iter = m_listProcess.begin(); iter != m_listProcess.end(); iter++ )
		{
			obj = *iter;

			obj->Lock();
			obj->m_refCount--;

			if ( !obj->ProcessRecvBuffer() || !obj->Recv() )
			{
				m_parent->ClosePeer( obj );
				continue;
			}

			obj->Unlock();
		}

		m_parent->OnEndDispatch();

		m_listProcess.clear();
	}
}
