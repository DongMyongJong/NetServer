// NetServer.h: interface for the NetServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETSERVER_H__BCBC8670_3C10_4397_86DF_816C300A767C__INCLUDED_)
#define AFX_NETSERVER_H__BCBC8670_3C10_4397_86DF_816C300A767C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment( lib, "ws2_32.lib" )

#include <stdio.h>
#include <WINSOCK2.H>
#include <windows.h>
#include "TcpServer.h"

using namespace std;

BOOL __stdcall BreakHandler( DWORD ctrlType );

typedef struct _PACKET 
{
	int device;
	int channel;
	int section;
	int sensor;
	int status;
} PACKET, *PPACKET;

class NetServer : public TcpServer, public Monitor
{
public:
	NetServer();

	TcpPeer *OnAccept( TcpListener *listener, SOCKET sock );
	void OnClosePeer(TcpPeer *obj);
	bool Run();
private:
	TcpListener m_listener3232;
	TcpListener m_listener9696;
};

class NetClient : public TcpPeer
{
public:
	virtual int OnExtractPacket( char *p, int len );
	virtual bool OnRecvComplete( char *p, int len );
private:
	void packetFromString(const char *src, PPACKET dst);
};

static HANDLE s_breakEvent;
static NetServer s_Server;
static class MySQLProxy *s_mySQLProxy;

#endif // !defined(AFX_NETSERVER_H__BCBC8670_3C10_4397_86DF_816C300A767C__INCLUDED_)
