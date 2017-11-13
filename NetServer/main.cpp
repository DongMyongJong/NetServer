#include "NetServer.h"

void main(int argc, char **argv)
{
	if (argc == 1)
	{
		s_Server.Start();
	} else {
		char *param = argv[1];
		if (strcmp(param, "install") == 0)
		{
			printf("Installing...\n");
			s_Server.Install();
			printf("Finished!\n");
			printf("Starting service...\n");
			if (s_Server.StartInstalledService())
			{
				printf("Starting service successfully!\n");
			} else {
				printf("Starting service failed!\n");
			}
		} else if (strcmp(param, "uninstall") == 0)
		{
			printf("Uninstalling...\n");
			s_Server.Uninstall();
			printf("Finished!\n");
		} 
	}
}