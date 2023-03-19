#pragma once
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <Windows.h>
#include <vector>
#include <process.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")

#define MAXBUFFER	1024
#define SERVER_PORT 3500

using namespace std;

enum class IOOperation
{
	RECV,
	SEND
};

// WSAOVERLAPPED ����ü�� Ȯ����� �ʿ��� �����߰�
struct stOverlappedEx
{
	WSAOVERLAPPED	m_wsaOverlapped;	// overlapped io ����ü
	SOCKET			m_socketClient;		// Ŭ���̾�Ʈ ����
	WSABUF			m_wsaBuf;			// overlapped io �۾�����
	char			m_szBuf[MAXBUFFER];	// ������ ����
	IOOperation		m_eOperaion;		// �۾� ���� ����
};

// Ŭ���̾�Ʈ ������ ��� ���� ����ü
struct stClientInfo
{
	SOCKET			m_socketClient;			// client �� ����Ǵ¼���
	stOverlappedEx	m_stRecvOverlappedEx;	// recv io �۾��� ���� ����
	stOverlappedEx	m_stSendOverlappedEx;	// send io �۾��� ���� ����

	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
		m_socketClient = INVALID_SOCKET;
	}
};

class IOCP
{
public:
	IOCP();
	~IOCP()
	{
		WSACleanup();
	}

	bool InitSocket();
	bool BindListen(int nBindPort);
	bool StartServer(const UINT32 maxClientCount);
	void DestroyThread();

	HANDLE GetCP() { return mIOCPHandle; }
	//bool GetIsWorker() { return mIsWorkerRun; }
private:
	void CreateClient(const UINT32 maxClientCount);
	bool CreateWorkerThread();	
	//bool CreatevWorkerThread();
	bool CreateAccepterThread();
	bool BindIOCompletionPort(stClientInfo* pClientInfo);
	bool BindRecv(stClientInfo* pClientInfo);
	bool SendMsg(stClientInfo* pClientInfo, char* pMsg, int nLen);

	stClientInfo* GetEmptyClientInfo();

	static unsigned int WINAPI WorkerThread(LPVOID lpParam);
	
	static unsigned int WINAPI AccepterThread(LPVOID hClientInfo);

	

	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false);

	// Ŭ���̾�Ʈ ���� ���� ����ü
	vector<stClientInfo> mClientInfos;
	// ���ӹޱ����� ���� ����
	SOCKET mListenSocket = INVALID_SOCKET;
	// ������ Ŭ���̾�Ʈ ��
	int mClientCnt = 0;
	// IO Worker ������
	HANDLE* mWorkerThread;
	// Accept ������
	HANDLE mAcceptThread;
	// CompletionPort ��ü �ڵ�
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;
	// �۾� ������ ���� �÷���
	bool mIsWorkerRun = true;
	// ���� ������ ���� �÷���
	bool mIsAcceptRun = true;
	// ���� ����
	char mSocketBuf[1024] = { 0 };

	// �����尹��
	int mThreadCount = 0;
	// ������ ���̵�
	//unsigned long threadId;
	// �ý��� ����
	SYSTEM_INFO mSystemInfo;

	// static bool mTestVar;
};

extern IOCP* g_pIOCP; // extern - for global frq 