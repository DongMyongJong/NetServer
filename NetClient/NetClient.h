// NetClient.h: interface for the NetClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETCLIENT_H__72C66534_ABFD_420C_8DB6_76F8DE63CA76__INCLUDED_)
#define AFX_NETCLIENT_H__72C66534_ABFD_420C_8DB6_76F8DE63CA76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment( lib, "ws2_32.lib" )

#include <stdio.h>
#include <WINSOCK2.H>
#include <windows.h>

class NetClient
{
public:
	NetClient();
	virtual ~NetClient();
	bool Connect(char *szSvrIpAddr, int nPort);
	void Disconnect();
	bool Send(char *p, int len);
private:
	SOCKET _sock;
};

#endif // !defined(AFX_NETCLIENT_H__72C66534_ABFD_420C_8DB6_76F8DE63CA76__INCLUDED_)
