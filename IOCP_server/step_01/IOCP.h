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

// WSAOVERLAPPED 구조체를 확장시켜 필요한 정보추가
struct stOverlappedEx
{
	WSAOVERLAPPED	m_wsaOverlapped;	// overlapped io 구조체
	SOCKET			m_socketClient;		// 클라이언트 소켓
	WSABUF			m_wsaBuf;			// overlapped io 작업버퍼
	char			m_szBuf[MAXBUFFER];	// 데이터 버퍼
	IOOperation		m_eOperaion;		// 작업 동작 종류
};

// 클라이언트 정보를 담기 위한 구조체
struct stClientInfo
{
	SOCKET			m_socketClient;			// client 와 연결되는소켓
	stOverlappedEx	m_stRecvOverlappedEx;	// recv io 작업을 위한 변수
	stOverlappedEx	m_stSendOverlappedEx;	// send io 작업을 위한 변수

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

	// 클라이언트 정보 저장 구조체
	vector<stClientInfo> mClientInfos;
	// 접속받기위한 리슨 소켓
	SOCKET mListenSocket = INVALID_SOCKET;
	// 접속한 클라이언트 수
	int mClientCnt = 0;
	// IO Worker 스레드
	HANDLE* mWorkerThread;
	// Accept 스레드
	HANDLE mAcceptThread;
	// CompletionPort 객체 핸들
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;
	// 작업 스레드 동작 플래그
	bool mIsWorkerRun = true;
	// 접속 스레드 동작 플래그
	bool mIsAcceptRun = true;
	// 소켓 버퍼
	char mSocketBuf[1024] = { 0 };

	// 스레드갯수
	int mThreadCount = 0;
	// 스레드 아이디
	//unsigned long threadId;
	// 시스템 인포
	SYSTEM_INFO mSystemInfo;

	// static bool mTestVar;
};

extern IOCP* g_pIOCP; // extern - for global frq 