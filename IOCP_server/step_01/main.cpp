#include "IOCP.h"

int main()
{
	g_pIOCP = new IOCP;

	g_pIOCP->InitSocket();

	g_pIOCP->BindListen(3500);

	g_pIOCP->StartServer(5);

	printf("key insert wait\n");
	getchar();

	g_pIOCP->DestroyThread();
	return 0;
}