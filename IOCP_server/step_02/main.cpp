#include "IocpManager.h"

int main()
{
	g_pIocpManager = new IocpManager;
	g_pSessionManager = new SessionManager;
	//PacketManager::GetInstance()->Init();
	g_pPacketManager = new PacketManager;

	if (g_pIocpManager->Init() == false)
		return -1;

	cout << "Init() Success" << endl;

	if (g_pIocpManager->StartWorkerThread() == false)
		return -1;

	cout << "WorkerThread Start()" << endl;
	
	if (g_pIocpManager->StartPacketProcessThread() == false)
		return -1;

	cout << "PacketProcessThread Start()" << endl;

	if (g_pIocpManager->AcceptLoop() == false)
		return -1;

	cout << "AcceptLoop() Start" << endl;

	g_pIocpManager->CloseIOCPServer();

	//PacketManager::GetInstance()->DestroyInstance();

	delete g_pSessionManager;
	delete g_pIocpManager;

	return 0;
}