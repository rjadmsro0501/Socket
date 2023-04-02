#include "IocpManager.h"

IocpManager* g_pIocpManager = nullptr;

IocpManager::IocpManager() : mCP(NULL), mThreadCount(2), mListenSocket(NULL)
{
}

IocpManager::~IocpManager()
{
}

bool IocpManager::Init()
{
	// read system info - for count of cpu core
	SYSTEM_INFO sysInfo;
	SecureZeroMemory(&sysInfo, sizeof(sysInfo));
	GetSystemInfo(&sysInfo);

	// count of thread 
	mThreadCount = sysInfo.dwNumberOfProcessors * 2;
	cout << "Thread Count : " << mThreadCount << endl;

	// init winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	// create completion port - first time
	mCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// except for create completion port
	if (mCP == NULL)
	{
		cout << "Completion Port Create Fail" << endl;
	}

	mListenSocket = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mListenSocket == NULL)
	{
		cout << "Socket Create Fail" << endl;
		return false;
	}

	// server addr setting
	SOCKADDR_IN serverAddr;
	SecureZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// setting socket option
	int opt = 1;
	setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));

	// bind listen socket
	if (SOCKET_ERROR == bind(mListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		cout << "Bind Fail" << endl;
		return false;
	}

	cout << "IOCP Server Init Success" << endl;

	return true;
}

bool IocpManager::StartServer()
{
	return false;
}

bool IocpManager::StartWorkerThread()
{
	// create thread
	for (int i = 0; i < mThreadCount; ++i)
	{
		DWORD dwThreadId;

		// begin thread
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (LPVOID)i, 0, (unsigned int*)&dwThreadId);
		
		// except error - for create thread
		if (hThread == INVALID_HANDLE_VALUE)
		{
			cout << "Create Worker Thread Fail" << endl;
			return false;
		}

		cout << "Create Thread No. " << i + 1 << endl;

		// close handle after create
		CloseHandle(hThread);
	}
	return true;
}

bool IocpManager::StartPacketProcessThread()
{
	// create thread
	DWORD dwThreadId;
	// begin thread
	HANDLE hPacketThread = (HANDLE)_beginthreadex(NULL, 0, PacketProcessThread, NULL, 0, (unsigned int*)&dwThreadId);

	// except error - for create thread 
	if (hPacketThread == INVALID_HANDLE_VALUE)
	{
		cout << "Create PacketProcess Thread Fail" << endl;
		return false;
	}

	cout << "Create Packet Thread" << endl;

	CloseHandle(hPacketThread);
	return true;
}

bool IocpManager::StartSystemUsageThread()
{
	// create thread
	DWORD dwThreadId;
	// begin thread
	HANDLE hSystemThread = (HANDLE)_beginthreadex(NULL, 0, SystemUsageThread, NULL, 0, (unsigned int*)&dwThreadId);
	if (hSystemThread == INVALID_HANDLE_VALUE)
	{
		cout << "Create Systme Thread Fail " << endl;
		return false;
	}

	cout << "Create System Thread" << endl;
	CloseHandle(hSystemThread);
	return true;
}

bool IocpManager::CloseIOCPServer()
{
	// close handle socket
	CloseHandle(mCP);
	WSACleanup();
	return true;
}

bool IocpManager::AcceptLoop()
{
	cout << "AcceptLoop() " << endl;
	// listen()
	if (SOCKET_ERROR == listen(mListenSocket, SOMAXCONN))
	{
		cout << "listen Fail" << endl;
		return false;
	}

	cout << "listen Success" << endl;

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);

	cout << "enter AcceptLoop" << endl;
	// accept loop
	while (true)
	{
		cout << "enter in loop" << endl;

		SOCKET acceptSocket = WSAAccept(mListenSocket, (sockaddr*)&clientAddr, &addrLen,NULL,NULL );
		cout << "Accept Socket" << endl;

		if (acceptSocket == INVALID_SOCKET)
		{
			cout << "accept fail socket" << endl;
			continue;
		}
		cout << "Accept Socket Success" << endl;

		ClientSession* client = g_pSessionManager->CreateClientSession(acceptSocket);

		if (false == client->OnConnect(&clientAddr))
		{
			cout << "IocpManager -> AcceptLoop() -> client -> OnConnect() fail " << endl;
			client->DisConnect();
			g_pSessionManager->DeleteClientSession(client);
		}

		client->Recv();
	}

	cout << "Check point" << endl;
	return true;
}



unsigned int __stdcall IocpManager::WorkerThread(LPVOID lpParam)
{
	HANDLE hCP = g_pIocpManager->GetCP();

	while (true)
	{
		DWORD dwBytesTransferred = 0;
		SOVERLAPPED* overlapped = nullptr;
		ClientSession* client = nullptr;

		bool ret = GetQueuedCompletionStatus(hCP, &dwBytesTransferred, (PULONG_PTR)&client, (LPOVERLAPPED*)&overlapped, INFINITE);
		
		if (ret == false && GetLastError() == WAIT_TIMEOUT)
		{
			cout << "Thread Time out" << endl;
			continue;
		}

		if (ret == false || dwBytesTransferred == 0)
		{
			cout << "Recv Data is zero " << endl;
			
			DisConnectCompletion(client, overlapped, dwBytesTransferred);
			g_pSessionManager->DeleteClientSession(client);
		}

		bool completionOk = true;

		switch (overlapped->mIOType)
		{
		case IOTYPE::IO_SEND :
			printf("Send Completion\n");
			completionOk = SendCompletion(client, overlapped, dwBytesTransferred);
			break;
		case IOTYPE::IO_RECV :
			printf("Recv Completion\n");
			completionOk = RecvCompletion(client, overlapped, dwBytesTransferred);
			break;
		case IOTYPE::IO_DISCONNECT :
			completionOk = DisConnectCompletion(client, overlapped, dwBytesTransferred);
			break;
		default:
			break;
		}

	}
	return 0;
}

unsigned int __stdcall IocpManager::PacketProcessThread(LPVOID lpParam)
{
	//PacketManager::GetInstance()->ProcessAllQueue();
	g_pPacketManager->ProcessAllQueue();
	printf("ProcessAllqueue Run()\n");
	return 0;
}

unsigned int __stdcall IocpManager::SystemUsageThread(LPVOID lpParam)
{

	return 0;
}

bool IocpManager::RecvCompletion(ClientSession* client, SOVERLAPPED* overlapped, DWORD dwBytesTransferr)
{
	if (client == nullptr)
	{
		cout << "Recv Client is nullptr" << endl;
		return false;
	}
	//PacketManager::GetInstance()->Enqueue(client, overlapped->mBuffer);
	g_pPacketManager->Enqueue(client, overlapped->mBuffer);
	return client->Recv();
}

bool IocpManager::SendCompletion(ClientSession* client, SOVERLAPPED* overlapped, DWORD dwBytesTransferr)
{
	if (client == nullptr)
	{
		cout << "SendCompletion client returned nullptr" << endl;
		return false;
	}

	if (overlapped->mWSABuf.len != dwBytesTransferr)
	{
		cout << "Send fail" << endl;
		return false;
	}
	return true;
}

bool IocpManager::DisConnectCompletion(ClientSession* client, SOVERLAPPED* overlapped, DWORD dwBytesTransferr)
{
	if (client == nullptr)
	{
		cout << "Disconnect client fail - client return nullptr" << endl;
		return false;
	}
	client->DisConnect();
	g_pSessionManager->DeleteClientSession(client);

	return true;
}
