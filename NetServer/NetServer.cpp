// NetServer.cpp: implementation of the NetServer class.
//
//////////////////////////////////////////////////////////////////////

#include "NetServer.h"
#include "MySQLProxy.h"
#include "iniFile.h"

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//-------------- NetServer -----------------------
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
NetServer::NetServer()
{
	TcpServer::TcpServer();
	m_szServiceName = "FANet";
}

bool NetServer::IsInstalled()
{
	bool bResult = false;
	
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
    if (hSCM != NULL)
    {
        SC_HANDLE hService = OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
		
		if (hService != NULL)
        {
            bResult = true;
            CloseServiceHandle(hService);
        }
        
		CloseServiceHandle(hSCM);
    }
	
    return bResult;
}

bool NetServer::Install()
{
	if (IsInstalled())
        return true;
	
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
    if (hSCM == NULL)
    {
        printf("Couldn't open service manager\n");
        return false;
    }
	
    // Get the executable file path
    char szFilePath[_MAX_PATH];
    
	GetModuleFileName(NULL, szFilePath, _MAX_PATH);
	
    SC_HANDLE hService = CreateService(hSCM, m_szServiceName, m_szServiceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, "MySQL\0", NULL, NULL);
	
    if (hService == NULL)
    {
        CloseServiceHandle(hSCM);
        printf("Couldn't create service!\n");
        return false;
    }
	
    CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
    return true;
}

bool NetServer::StartInstalledService()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
    {
        printf("Couldn't open service manager\n");
        return false;
    }
	SC_HANDLE hService = OpenService(hSCM, m_szServiceName, SERVICE_ALL_ACCESS);
	if (hService == NULL) 
    { 
        printf("Couldn't open service\n");
		return false;
    }
	if (!StartService(hService, 0, NULL) )
    {
        printf("Cannot Start Service!\n");
		return false;
    }
	return true;
}

bool NetServer::Uninstall()
{
	if (!IsInstalled())
        return true;
	
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
    if (hSCM == NULL)
    {
        printf("Couldn't open service manager\n");
        return false;
    }
	
    SC_HANDLE hService = OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);
	
    if (hService == NULL)
    {
        CloseServiceHandle(hSCM);
        printf("Couldn't open service\n");
        return false;
    }
	
    SERVICE_STATUS status;
    
	ControlService(hService, SERVICE_CONTROL_STOP, &status);
	
    bool bDelete = DeleteService(hService);
    
	CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
	
    if (bDelete)
        return true;
	
    printf("Service could not be deleted\n");
    return false;
}

void NetServer::Start()
{
	SERVICE_TABLE_ENTRY   DispatchTable[] = 
    { 
        { m_szServiceName, _ServiceMain}, 
        { NULL, NULL } 
    }; 
	
	if (!StartServiceCtrlDispatcher( DispatchTable)) {
		printf("Cannot start Service!\n");
	}
}

void WINAPI NetServer::_ServiceMain(DWORD argc, LPTSTR *argv)
{
	s_Server.ServiceMain(argc, argv);
}

void WINAPI NetServer::_Handler(DWORD dwOpcode)
{
	s_Server.Handler(dwOpcode);
}

void NetServer::ServiceMain(DWORD argc, LPTSTR *argv)
{
    m_serviceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS; 
    m_serviceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    m_serviceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP; 
    m_serviceStatus.dwWin32ExitCode      = 0; 
    m_serviceStatus.dwServiceSpecificExitCode = 0; 
    m_serviceStatus.dwCheckPoint         = 0; 
    m_serviceStatus.dwWaitHint           = 0; 
	
    m_serviceStatusHandle = RegisterServiceCtrlHandler( m_szServiceName, _Handler); 
	
    if (m_serviceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    { 
        printf("RegisterServiceCtrlHandler failed\n"); 
		return; 
    } 

	if (!Run())
	{
		printf("Running server failed!\n");
	}

	// Initialization complete - report running status. 
    m_serviceStatus.dwCurrentState       = SERVICE_RUNNING; 
    m_serviceStatus.dwCheckPoint         = 0; 
    m_serviceStatus.dwWaitHint           = 0; 
	
    if (!SetServiceStatus (m_serviceStatusHandle, &m_serviceStatus)) 
    { 
        printf("SetServiceStatus error\n"); 
    }

	printf("Returning the Main Thread \n",0); 

    return; 
}

void NetServer::Handler(DWORD dwOpcode)
{
	switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
		m_listener.Uninit();
		Uninit();
		delete s_mySQLProxy;

		m_serviceStatus.dwWin32ExitCode = 0; 
		m_serviceStatus.dwCurrentState  = SERVICE_STOPPED; 
		m_serviceStatus.dwCheckPoint    = 0; 
		m_serviceStatus.dwWaitHint      = 0; 
        break;
	default: 
		printf("Unrecognized opcode %ld\n", dwOpcode); 
    }
	// Send current status. 
    if (!SetServiceStatus (m_serviceStatusHandle,  &m_serviceStatus)) 
    { 
        printf("SetServiceStatus error!\n"); 
    } 
    return; 
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
	char path[_MAX_PATH], iniPath[_MAX_PATH], logPath[_MAX_PATH];
	char drive[512], dir[512], fname[512], ext[512];
	GetModuleFileName(NULL, path, _MAX_PATH);
	_splitpath( path, drive, dir, fname, ext );
	_makepath( iniPath, drive, dir, "config", "ini" );
	_makepath( logPath, drive, dir, "fanet", "log" );
	CIniFile iniFile(iniPath);
	iniFile.ReadFile();
	s_logFile = fopen(logPath, "a");
	SYSTEMTIME tm; GetSystemTime(&tm);
	fprintf(s_logFile, "----- %d / %d / %d ---- %d : %d : %d ----------\n", tm.wDay, tm.wMonth, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond);
	int port = iniFile.GetValueI("server", "port");
	string ip = iniFile.GetValue("mysql", "ip");
	int  mySQLPort = iniFile.GetValueI("mysql", "port");
	string db = iniFile.GetValue("mysql", "db");
	string user = iniFile.GetValue("mysql", "user");
	string pwd = iniFile.GetValue("mysql", "pwd");
	
	fprintf(s_logFile, "Initializing NetServer...\n" );
	if (Init()) {
		fprintf(s_logFile, "Successed Initializing NetServer!\n" );
	} else {
		fprintf(s_logFile, "Failed initializing NetServer!\n" );
		return false;
	}
	
	fprintf(s_logFile, "Initializing MySQL server...\n" );
	s_mySQLProxy = new MySQLProxy();
	if (s_mySQLProxy->init(db.c_str(), ip.c_str(), mySQLPort, user.c_str(), pwd.c_str()))
	{
		fprintf(s_logFile, "Successed Initializing MySQL server...\n" );
	} else {
		fprintf(s_logFile, "Failed Initializing MySQL server...\n" );
		return false;
	}
	
	fprintf(s_logFile, "Initializing New TcpListener(%d)...\n", port );
	if ( !m_listener.Init( SockAddr( port ) ) )
	{
		fprintf(s_logFile, "Err! Failed to initialize TcpListener(%d).\n", port );
		Uninit();
		delete s_mySQLProxy;
		return false;
	} else {
		fprintf(s_logFile, "Successed Initializing New TcpListener(%d)...\n", port );
	}
	
	fprintf(s_logFile, "Starting TcpListener...\n" );
	if (Listen( &m_listener )) {
		fprintf(s_logFile, "Successed Starting TcpListener!\n" );
	} else {
		fprintf(s_logFile, "Failed Starting TcpListener!\n" );
		return false;
	}
	
	fprintf(s_logFile, "NetServer started!\n" );
	fclose(s_logFile);

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