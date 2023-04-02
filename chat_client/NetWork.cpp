#include "NetWork.h"

SOCKET NetWork::mClientSocket = NULL;
NetWork* g_pNetWork = nullptr;

NetWork::NetWork()
{
}

NetWork::~NetWork()
{
	Clean();
}

void NetWork::Init()
{
	WSAStartup(MAKEWORD(2, 2), &mWSAData);
	Set_TCPSocket();
}

void NetWork::Update()
{
}

void NetWork::Clean()
{
	closesocket(mClientSocket);
	WSACleanup();
}

void NetWork::Set_TCPSocket()
{
	cout << "Set Socket()" << endl;

	mClientSocket = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	if (mClientSocket == INVALID_SOCKET)
		perror("socket() error");
}

void NetWork::Connect(short portNum)
{
	cout << "connect()" << endl;

	SOCKADDR_IN serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNum);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

	if (connect(mClientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << WSAGetLastError() << endl;
		perror("connect() error");
	}
}

void NetWork::Run()
{
	Init();
	Connect(SERVER_PORT);
	StartRecvThread();
	StartPacketProcessThread();
}

bool NetWork::StartRecvThread()
{
	DWORD dwThreadId;
	HANDLE hRecvThread = (HANDLE)_beginthreadex(NULL, 0, recvThread, NULL, 0, (unsigned int*)&dwThreadId);

	if (hRecvThread == INVALID_HANDLE_VALUE)
	{
		cout << "Create RecvThread fail" << endl;
		return false;
	}

	cout << "Create RecvThread " << endl;
	CloseHandle(hRecvThread);
	return true;
}

bool NetWork::StartPacketProcessThread()
{
	DWORD dwThreadId;
	HANDLE hPacketProcessThread = (HANDLE)_beginthreadex(NULL, 0, PacketProcessThread, NULL, 0, (unsigned int*)&dwThreadId);
	if (hPacketProcessThread == INVALID_HANDLE_VALUE)
	{
		cout << "Create PacketProcess Thread fail" << endl;
		return false;
	}
	
	cout << "Create PacketProcess" << endl;
	CloseHandle(hPacketProcessThread);

	return true;
}

bool NetWork::SendPacket(PROTOCOL protocol, char* data, DWORD dataSize, bool inGame)
{
	// head create
	SHEAD head;
	memset(&head, 0, sizeof(SHEAD));

	head.mCmd = (unsigned char)protocol;
	head.mPacketSize = (DWORD)sizeof(SHEAD) + dataSize;
	head.mTransferToInGame = inGame;
	
	// head + data
	char buffer[MAXBUFFER];

	int bufferSize = head.mPacketSize;
	memset(buffer, 0, MAXBUFFER);
	memcpy(buffer, (char*)&head, sizeof(SHEAD));

	if (data != NULL)
	{
		memcpy(buffer + sizeof(SHEAD), data, dataSize);
	}

	// packet send
	if (SOCKET_ERROR == send(mClientSocket, buffer, bufferSize, 0))
	{
		cout << "send packet error" << endl;
		return false;
	}
	return true;
}

unsigned int __stdcall NetWork::recvThread(LPVOID lpParam)
{
	while (true)
	{
		char* recvBuffer = new char[MAXBUFFER];
		memset(recvBuffer, 0, MAXBUFFER);
		if (SOCKET_ERROR == recv(mClientSocket, recvBuffer, MAXBUFFER, 0))
		{
			cout << "recv packet error : "<<WSAGetLastError() << endl;
			return false;
		}

		if (g_pPacketManager != nullptr)
		{
			g_pPacketManager->Enqueue(recvBuffer);
		}
	}
	return 0;
}

unsigned int __stdcall NetWork::PacketProcessThread(LPVOID lpParam)
{
	if (g_pPacketManager != nullptr)
	{
		g_pPacketManager->ProcessAllQueue();
	}
	return 0;
}
