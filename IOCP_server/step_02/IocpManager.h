#pragma once
#include "stdafx.h"
#include "Packet.h"
#include "PacketManager.h"
#include "ClientSession.h"
#include "SessionManager.h"
#include "TemplateSingleton.h"

class IocpManager
{
public:
	IocpManager();
	~IocpManager();

	bool Init();	
	bool StartServer();
	
	bool StartWorkerThread();			// WorkerThread()
	bool StartPacketProcessThread();	// PacketProcessThread()
	bool StartSystemUsageThread();		// SystemUsageThread()

	bool CloseIOCPServer();
	bool AcceptLoop();

	//void DestroyThread();

	HANDLE GetCP() { return mCP; }

private:

	HANDLE mCP;
	SOCKET mListenSocket;

	bool mIsConnected;
	int mThreadCount;
	int mClientCount;
	
	static unsigned int WINAPI WorkerThread(LPVOID lpParam);		// GetQueuedCompletionStatus()
	static unsigned int WINAPI PacketProcessThread(LPVOID lpParam); // PacketManager()->ProcessAllQueue() 
	static unsigned int WINAPI SystemUsageThread(LPVOID lpParam);

	static bool RecvCompletion(ClientSession* client, SOVERLAPPED* overlapped, DWORD dwBytesTransferr);
	static bool SendCompletion(ClientSession* client, SOVERLAPPED* overlapped, DWORD dwBytesTransferr);
	static bool DisConnectCompletion(ClientSession* client, SOVERLAPPED* overlapped, DWORD dwBytesTransferr);

};

extern IocpManager* g_pIocpManager;

// ServerInit
// Start Thread funcs
// IOCPThread
// PacketProcessThread
// SystemUsageThread;
// AcceptLoop
// Get - CP, ThreadCount, ClientCount
// private var : CP, ThreadCount, ListenSocket

// ThreadFunc List
// WorkerThread
// PacketProcessThread
// SystemUsageProcess - log, SystemInfo

// Completion Func
// Recv
// Send
// Disconnect