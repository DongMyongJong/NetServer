#include "NetClient.h"
#include <time.h>

HANDLE g_breakEvent;

BOOL __stdcall BreakHandler( DWORD ctrlType )
{
	SetEvent( g_breakEvent );
	
	return TRUE;
}
void main()
{
	NetClient client;
	if (!client.Connect("127.0.0.1", 3232))
	{
		printf("Failed connecting to server(3232)\n");
	} else {
		printf("Successed server connection!\n");
		char data1[] = "100|99|98|1|1|";
		char data2[] = "100|99|98|4|2|";
		client.Send(data1, strlen(data1));
		client.Send(data2, strlen(data2));
	}

	//if (!client.Connect("127.0.0.1", 9696))
	//{
	//	printf("Failed connecting to server(9696)\n");
	//} else {
	//	printf("Successed server connection!\n");
	//	char data1[] = "1|9|98|97|96|";
	//	char data2[] = "1|9|98|100|100|";
	//	char data3[] = "1|9|12|90|90|";
	//	char data4[] = "1|10|12|90|90|";
	//	client.Send(data1, strlen(data1));
	//	client.Send(data2, strlen(data2));
	//	client.Send(data3, strlen(data3));
	//	client.Send(data4, strlen(data4));
	//}

	g_breakEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	printf("Waiting...\n");
	SetConsoleCtrlHandler( BreakHandler, TRUE );
	{
		WaitForSingleObject( g_breakEvent, INFINITE );
		CloseHandle( g_breakEvent );
	}
	SetConsoleCtrlHandler( BreakHandler, FALSE );
	printf("Disconnecting...\n");

	client.Disconnect();
}
