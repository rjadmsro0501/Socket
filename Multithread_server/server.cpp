#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <process.h>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")

#define MAX_BUFFER		1024
#define SERVER_PORT		3500
#define BACK_LOG_SIZE	5

SOCKET SetTcpServer(short pnum, int blog);
void AcceptLoop(SOCKET _socket);
void DoIt(void* param);

int main()
{
	WSADATA wsadata; 
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	SOCKET listenSocket = SetTcpServer(SERVER_PORT, BACK_LOG_SIZE);
	if (listenSocket == SOCKET_ERROR)
	{
		printf("Set Tcp Socket Error\n");
		WSACleanup();
		return 0;
	}

	AcceptLoop(listenSocket);
	WSACleanup();
	
	return 0;
}

SOCKET SetTcpServer(short pnum, int blog)
{
	SOCKET setSocket;
	setSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (setSocket == -1)
	{
		return -1;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	int retval = 0;
	retval = bind(setSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (retval == -1) { return -1; }

	retval = listen(setSocket, blog);
	if (retval == -1) { return -1; }

	return setSocket;
}

void AcceptLoop(SOCKET _socket)
{
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	memset(&clientAddr, 0, sizeof(clientAddr));

	while (true)
	{
		clientSocket = accept(_socket,(sockaddr*)&clientAddr,&addrLen);
		if (clientSocket == -1)
		{
			printf("Accept Fail\n");
			break;
		}

		printf("%s : %d 의 연결 요청 수락\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		_beginthread(DoIt, 0, (void*)clientSocket);
	}

	closesocket(_socket);
}

void DoIt(void* param)
{
	SOCKET doSock = (SOCKET)param;

	SOCKADDR_IN clientAddr;
	int addrLen;
	getpeername(doSock,(sockaddr*)&clientAddr,&addrLen);
	char msg[MAX_BUFFER] = "";
	while (recv(doSock, msg, sizeof(msg), 0) > 0)
	{
		printf("%s : %d [user %d]recv : %s\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),(int)doSock,msg);
		
		send(doSock, msg, sizeof(msg), 0);
	}

	printf("%s : %d [user %d] 통신종료\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port),(int)doSock);

	closesocket(doSock);
}
