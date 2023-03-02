#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER	1024
#define SERVER_PORT 3500

struct SOCKETINFO
{
    WSAOVERLAPPED overlapped;
    WSABUF dataBuffer;
    SOCKET socket;
    char messageBuffer[MAX_BUFFER];
    int receiveBytes;
    int sendBytes;
};

void CALLBACK callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD InFlags);


int main()
{
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
        printf("Error Can not load winsock \n"); 

    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET)
        printf("Error Invalid Socket\n"); 

    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    int retval = 0;
    retval = bind(listenSocket, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
    if (retval == SOCKET_ERROR)
    {
        printf("Error Fail bind\n");
        //EndSocket(listenSocket);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    retval = listen(listenSocket, 5);
    if (retval == SOCKET_ERROR)
    {
        printf("Error Fail listen\n");
       //EndSocket(listenSocket);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);
    memset(&clientAddr, 0, addrLen);

    SOCKET clientSocket;
    SOCKETINFO* socketInfo;
    DWORD recvBytes;
    DWORD flags;

    while (1)
    {
        clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            printf("Error Accept Fail\n");
            return 1;
        }

        socketInfo = new SOCKETINFO;
        memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));

        socketInfo->socket = clientSocket;
        socketInfo->dataBuffer.len = MAX_BUFFER;
        socketInfo->dataBuffer.buf = socketInfo->messageBuffer;
        flags = 0;

        // 중첩 소켓을 지정하고 완료시 실행될 함수를 넘겨준다
        if (WSARecv(socketInfo->socket,
            &socketInfo->dataBuffer,
            1,
            &recvBytes,
            &flags,
            &(socketInfo->overlapped),
            callback))
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("Error IO pending Fail\n");
                return 1;
            }
        }
    }

    closesocket(listenSocket);
    WSACleanup();

   // EndSocket(listenSocket);

	return 0;
}

void CALLBACK callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD InFlags)
{
    struct SOCKETINFO* socketInfo;
    DWORD sendBytes = 0;
    DWORD recvBytes = 0;
    DWORD flags = 0;

    socketInfo = (struct SOCKETINFO*)overlapped;
    memset(&(socketInfo->overlapped), 0x00, sizeof(WSAOVERLAPPED));
    
    if (dataBytes == 0)
    {
        closesocket(socketInfo->socket);
        free(socketInfo);

        return;
    }

    if (socketInfo->receiveBytes == 0)
    {
        // WSARecv(최초 대기에 대한)의 콜백일 경우
        socketInfo->receiveBytes = dataBytes;
        socketInfo->sendBytes = 0;
        socketInfo->dataBuffer.buf = socketInfo->messageBuffer;
        socketInfo->dataBuffer.len = socketInfo->receiveBytes;

        printf("recv : %s (%d bytes)\n", socketInfo->messageBuffer, dataBytes);

        if (WSASend(socketInfo->socket,
            &(socketInfo->dataBuffer),
            1, &sendBytes,
            0,
            &(socketInfo->overlapped),
            callback) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("Error - Fail WSASend(error_code : %d)\n",WSAGetLastError());
            }
        }
    }
    else
    {
        // WSASend(응답에 대한)의 콜백일 경우
        socketInfo->sendBytes += dataBytes;
        socketInfo->receiveBytes = 0;
        socketInfo->dataBuffer.len = MAX_BUFFER;
        socketInfo->dataBuffer.buf = socketInfo->messageBuffer;

        printf("Send : %s (%d bytes)\n", socketInfo->messageBuffer, dataBytes);
        if (WSARecv(socketInfo->socket,
            &socketInfo->dataBuffer,
            1,
            &recvBytes,
            &flags,
            &(socketInfo->overlapped),
            callback) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("Error Fail WSARecv(error_code : %d)\n", WSAGetLastError());
            }
        }

    }
}

