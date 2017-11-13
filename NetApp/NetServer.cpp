// NetServer.cpp: implementation of the NetServer class.
//
//////////////////////////////////////////////////////////////////////

#include "NetServer.h"
#include "MySQLProxy.h"
#include "iniFile.h"

BOOL __stdcall BreakHandler( DWORD ctrlType )
{
	SetEvent( s_breakEvent );
	
	return TRUE;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//-------------- NetServer -----------------------
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
NetServer::NetServer()
{
	TcpServer::TcpServer();
}

TcpPeer *NetServer::OnAccept( TcpListener *listener, SOCKET sock )
{
	NetClient *obj = new NetClient();
	obj->LinkAcceptedSocket(sock);
	return obj;
}

void NetServer::OnClosePeer(TcpPeer *obj)
{
	s_mySQLProxy->writeDeviceStatus(obj->_device, false);
}

bool NetServer::Run()
{
	printf( "Reading config file...\n" );
	char path[_MAX_PATH], iniPath[_MAX_PATH];
	char drive[512], dir[512], fname[512], ext[512];
	GetModuleFileName(NULL, path, _MAX_PATH);
	_splitpath( path, drive, dir, fname, ext );
	_makepath( iniPath, drive, dir, "config", "ini" );
	CIniFile iniFile(iniPath);
	iniFile.ReadFile();
	int port = iniFile.GetValueI("server", "port");
	string ip = iniFile.GetValue("mysql", "ip");
	int  mySQLPort = iniFile.GetValueI("mysql", "port");
	string db = iniFile.GetValue("mysql", "db");
	string user = iniFile.GetValue("mysql", "user");
	string pwd = iniFile.GetValue("mysql", "pwd");
	
	printf( "Initializing NetServer...\n" );
	Init();
	
	printf( "Initializing MySQL server...\n" );
	s_mySQLProxy = new MySQLProxy(db.c_str(), ip.c_str(), mySQLPort, user.c_str(), pwd.c_str());
	
	port = 3232;
	printf( "Initializing New TcpListener(%d)...\n", port );
	if ( !m_listener3232.Init( SockAddr( port ) ) )
	{
		printf( "Err! Failed to initialize TcpListener(%d).\n", port );
		Uninit();
		delete s_mySQLProxy;
		return false;
	}
	
	printf( "Starting TcpListener(%d)...\n", port );
	Listen( &m_listener3232 );
	
	port = 9696;
	printf( "Initializing New TcpListener(%d)...\n", port );
	if ( !m_listener9696.Init( SockAddr( port ) ) )
	{
		printf( "Err! Failed to initialize TcpListener(%d).\n", port );
		Uninit();
		delete s_mySQLProxy;
		return false;
	}

	printf( "Starting TcpListener(%d)...\n", port );
	Listen( &m_listener9696 );

	printf( "NetServer initialized!\n" );

	s_breakEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	SetConsoleCtrlHandler( BreakHandler, TRUE );
	{
		WaitForSingleObject( s_breakEvent, INFINITE );
		CloseHandle( s_breakEvent );
	}
	SetConsoleCtrlHandler( BreakHandler, FALSE );
	
	printf( "Finalzing EchoServer...\n" );
	m_listener3232.Uninit();
	m_listener9696.Uninit();
	Uninit();

	return true;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//-------------- NetClient ----------------------
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
int NetClient::OnExtractPacket( char *p, int len ) 
{ 
	return len; 
}

bool NetClient::OnRecvComplete( char *p, int len ) 
{ 	
	PACKET packet;
	packetFromString(p, &packet);
	_device = packet.device;
	printf("Writing Sensor data and device status(%s)...\n", p);
	s_mySQLProxy->writeSensorData(&packet);
	s_mySQLProxy->writeDeviceStatus(_device, true);
	return true; 
}

void NetClient::packetFromString(const char *src, PPACKET dst)
{
	int len = strlen(src);
	int _start = 0;
	int _key = 0;
	for (int i = 0; i < len; i++)
	{
		if (src[i] == '|')
		{
			char tmp[512]; memset(tmp, 0, 512);
			strncpy(tmp, src + _start, i - _start);
			switch(_key) 
			{
			case 0: // device
				dst->device = atoi(tmp);
				break;
			case 1: // channel
				dst->channel = atoi(tmp);
				break;
			case 2: // section
				dst->section = atoi(tmp);
				break;
			case 3: // sensor
				dst->sensor = atoi(tmp);
				break;
			case 4: // status
				dst->status = atoi(tmp);
				break;
			default:
				return;
			}
			_start = i + 1;
			_key++;
		}
	}
}