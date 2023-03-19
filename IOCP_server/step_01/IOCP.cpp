#include "IOCP.h"

IOCP* g_pIOCP = nullptr;

IOCP::IOCP()
{
    GetSystemInfo(&mSystemInfo);
    mThreadCount = mSystemInfo.dwNumberOfProcessors * 2;
   // mTestVar = true;
}

bool IOCP::InitSocket()
{
    WSADATA wsaData;
    int nRet = WSAStartup(MAKEWORD(2,2),&wsaData);
    if (nRet != 0)
    {
        printf("WSAStart Error\n");
        return false;
    }

    // 리슨 소켓 설정
    mListenSocket = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,NULL,WSA_FLAG_OVERLAPPED);
    if (mListenSocket == INVALID_SOCKET)
    {
        printf("Socket Create Error\n");
        return false;
    }
    printf("Socket Create Success\n");

    return true;
}

bool IOCP::BindListen(int nBindPort)
{
    SOCKADDR_IN serverAddr;

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(nBindPort);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    int nRet = bind(mListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (nRet != 0)
    {
        printf("Bind Error : code %d \n", WSAGetLastError());
        return false;
    }

    nRet = listen(mListenSocket, 5);
    if (nRet != 0)
    {
        printf("Listen Error : code %d \n", WSAGetLastError());
        return false;
    }

    printf("Bind & Listen Success\n");
    return true;
}

bool IOCP::StartServer(const UINT32 maxClientCount)
{
    CreateClient(maxClientCount);

    mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    if (mIOCPHandle == NULL)
    {
        printf("CreateIoCompletionPort() Error : code %d\n", GetLastError());
        return false;
    }

    bool bRet = CreateWorkerThread();
    if (bRet == false)
    {
        return false;
    }

    bRet = CreateAccepterThread();
    if (bRet == false)
    {
        return false;
    }

    return true;
}

void IOCP::DestroyThread()
{
}

void IOCP::CreateClient(const UINT32 maxClientCount)
{
    for (UINT i = 0; i < maxClientCount; i++)
    {
        mClientInfos.emplace_back();
    }
}

bool IOCP::CreateWorkerThread()
{
    unsigned int uiThreadId = 0;
    mWorkerThread = (HANDLE*)malloc(mThreadCount * sizeof(HANDLE));

    for (int i = 0; i < mThreadCount; i++)
    {
        mWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (LPVOID)i, 0, &uiThreadId);
        //mWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, this->WorkerThread, this, 0, &uiThreadId);
        //mWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, &uiThreadId);
    
    }
    return true;
}



//bool IOCP::CreatevWorkerThread()
//{
//    unsigned int uid = 0;
//    typedef unsigned(__stdcall* WorkerThreadProc)(LPVOID);
//    _beginthreadex_proc_type pWorkerThread = (_beginthreadex_proc_type)&IOCP::vWorkerThread;
//   // WorkerThreadProc pWorkerThread = &IOCP::vWorkerThread;
//    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, pWorkerThread, NULL, 0, &uid);
//    return true;
//}

bool IOCP::CreateAccepterThread()
{
    unsigned int uiThreadId = 0; 
    mAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AccepterThread, NULL, 0, &uiThreadId);
    //mAcceptThread = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, &uiThreadId);
    return true;
}

bool IOCP::BindIOCompletionPort(stClientInfo* pClientInfo)
{
    HANDLE hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient,
        mIOCPHandle,
        (ULONG_PTR)(pClientInfo), 0);

    if (hIOCP == NULL || mIOCPHandle != hIOCP)
    {
        printf("CreateIoCompletionPort() Error : code %d \n", GetLastError());
        return false;
    }
    return true;
}

bool IOCP::BindRecv(stClientInfo* pClientInfo)
{
    DWORD dwFlag = 0;
    DWORD dwRecvNumBytes = 0;

    pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAXBUFFER;
    pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stRecvOverlappedEx.m_szBuf;
    pClientInfo->m_stRecvOverlappedEx.m_eOperaion = IOOperation::RECV;

    int nRet = WSARecv(pClientInfo->m_socketClient,
        &(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
        1,
        &dwRecvNumBytes,
        &dwFlag,
        (LPWSAOVERLAPPED) & (pClientInfo->m_stRecvOverlappedEx),
        NULL);
    if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
    {
        printf("WSARecv() Error : code %d\n", WSAGetLastError());
        return false;
    }

    return true;
}

bool IOCP::SendMsg(stClientInfo* pClientInfo, char* pMsg, int nLen)
{
    DWORD dwSendNumBytes = 0;

    CopyMemory(pClientInfo->m_stSendOverlappedEx.m_szBuf, pMsg, nLen);

    pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = nLen;
    pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stSendOverlappedEx.m_szBuf;
    pClientInfo->m_stSendOverlappedEx.m_eOperaion = IOOperation::SEND;

    int nRet = WSASend(pClientInfo->m_socketClient,
        &(pClientInfo->m_stSendOverlappedEx.m_wsaBuf),
        1,
        &dwSendNumBytes,
        0,
        (LPWSAOVERLAPPED) & (pClientInfo->m_stSendOverlappedEx),
        NULL);

    return true;
}

stClientInfo* IOCP::GetEmptyClientInfo()
{
    for (auto& client : mClientInfos)
    {
        if (INVALID_SOCKET == client.m_socketClient)
        {
            return &client;
        }
    }

    /*C Style
    for (size_t i = 0; i < mClientInfos.size(); i++)
    {
        if (INVALID_SOCKET == mClientInfos[i].m_socketClient)
        {
            return &mClientInfos[i];
        }
    }*/
    return nullptr;
}

unsigned int WINAPI IOCP::WorkerThread(LPVOID lpParam)
{
    stClientInfo* pClientInfo = NULL;
    bool bSuccess = true;
    DWORD dwIoSize = 0;
    LPOVERLAPPED lpOverlapped = NULL;  

    // IOCP* This = (IOCP*)lpParam;
    // This->mIsWorkerRun = true;
    // 
    // 비정적 멤버변수를 정적멤버함수에서 접근하려할경우 에러 발생
    // while(mIsWorker) -> while(g_pIOCP->mIsWorkerRun)
    // while(This->mIsWorkerRun)
    // 
    // GetQueuedCompletionStatus(mIOCPHandle)-> 
    // HANDLE hCP = g_pIOCP->GetCP(); ->
    // GetQueuedCompletionStatus(hCP,)
    HANDLE hCP = g_pIOCP->GetCP();      
    
    while (g_pIOCP->mIsWorkerRun)
    {

        bSuccess = GetQueuedCompletionStatus(hCP,
                  &dwIoSize,
                  (PULONG_PTR)&pClientInfo, // CompletionKey
                  &lpOverlapped,
                  INFINITE);

        if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
        {
            g_pIOCP->mIsWorkerRun = false;
            continue;
        }
        if (NULL == lpOverlapped)
        {
            continue;
        }

        if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess))
        {
            printf("socket(%d) logout \n", (int)pClientInfo->m_socketClient);
            g_pIOCP->CloseSocket(pClientInfo);
            continue;
        }

        stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

        // Overlapped IO Recv 작업 결과 뒤 처리
        if (IOOperation::RECV == pOverlappedEx->m_eOperaion)
        {
            pOverlappedEx->m_szBuf[dwIoSize] = NULL;
            printf("[RECV] bytes : %d , msg : %s \n", dwIoSize, pOverlappedEx->m_szBuf);

            //Echo
            g_pIOCP->SendMsg(pClientInfo, pOverlappedEx->m_szBuf, dwIoSize);
            g_pIOCP->BindRecv(pClientInfo);
        }
        else if (IOOperation::SEND == pOverlappedEx->m_eOperaion)
        {
            printf("[SEND] bytes : %d , msg : %s \n", dwIoSize, pOverlappedEx->m_szBuf);

        }
        else
        {
            printf("socket(%d) Except Error\n",(int)pClientInfo->m_socketClient);
        }

    }
    
    return 0;
}

unsigned int WINAPI IOCP::AccepterThread(LPVOID hClientInfo)
{
    SOCKADDR_IN stClientAddr;
    int nAddrLen = sizeof(SOCKADDR_IN);

    while (g_pIOCP->mIsAcceptRun)
    {
        stClientInfo* pClientInfo = g_pIOCP->GetEmptyClientInfo();
        if (pClientInfo == NULL)
        {
            printf("Error Client Full");
            return 0;
        }

        pClientInfo->m_socketClient = accept(g_pIOCP->mListenSocket, (sockaddr*)&stClientAddr, &nAddrLen);
        if (INVALID_SOCKET == pClientInfo->m_socketClient)
        {
            continue;
        }

        bool bRet = g_pIOCP->BindIOCompletionPort(pClientInfo);
        if (bRet == false)
        {
            return 0;
        }

        bRet = g_pIOCP->BindRecv(pClientInfo);
        if (bRet == false)
        {
            return 0;
        }

        char clientIP[32] = { 0, };
        inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
        printf("Client Accept : IP (%s) Socket(%d)\n", clientIP, (int)pClientInfo->m_socketClient);
        g_pIOCP->mClientCnt++;
    }

    return 0;
}

void IOCP::CloseSocket(stClientInfo* pClientInfo, bool bIsForce)
{
    struct linger stLinger = { 0, 0 };	// SO_DONTLINGER로 설정

    // bIsForce가 true이면 SO_LINGER, timeout = 0으로 설정하여 강제 종료 시킨다. 주의 : 데이터 손실이 있을수 있음 
    if (true == bIsForce)
    {
        stLinger.l_onoff = 1;
    }

    //socketClose소켓의 데이터 송수신을 모두 중단 시킨다.
    shutdown(pClientInfo->m_socketClient, SD_BOTH);

    //소켓 옵션을 설정한다.
    setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

    //소켓 연결을 종료 시킨다. 
    closesocket(pClientInfo->m_socketClient);

    pClientInfo->m_socketClient = INVALID_SOCKET;
}
