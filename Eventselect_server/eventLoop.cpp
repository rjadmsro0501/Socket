#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"Ws2_32.lib")
#define MAX_BUFFER	1024	
#define SERVER_PORT 3500
#define BLOG		5	

struct SOCKEINFO
{
	SOCKET socket;
	int recvBytes;
	int sendBytes;
	char msgBuffer[MAX_BUFFER];
};

SOCKET SetTcpServer(short _pnum, int _backlog);
void EventLoop(SOCKET _socket);

HANDLE AddNetWorkEvent(SOCKET _socket, long _event);

SOCKET sockArr[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];

void AcceptProc(int _index);
void ReadProc(int _index);
void CloseProc(int _index);


int totalIndex = 0;

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	
	SOCKET initSocket;
	initSocket = SetTcpServer(SERVER_PORT, BLOG);

	if (initSocket == -1)
	{
		printf("Socket Error \n");
	}
	else 
	{
		EventLoop(initSocket);
	}
	
	WSACleanup();
	return 0;
}

SOCKET SetTcpServer(short _pnum, int _backlog)
{
	SOCKET listenSocket;
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == SOCKET_ERROR)
	{
		printf("Create Socket Fail\n");
		return -1;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(_pnum);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		printf("Bind Fail\n");
		return -1;
	}

	if (listen(listenSocket, _backlog) == -1)
	{
		printf("Listen Fail\n");
		return -1;
	}

	return listenSocket;
}

void EventLoop(SOCKET _socket)
{
	AddNetWorkEvent(_socket, FD_ACCEPT);

	while (true)
	{
		int index = WSAWaitForMultipleEvents(totalIndex, hEventArr, FALSE, WSA_INFINITE, FALSE);
		
		WSANETWORKEVENTS netEvents;
		WSAEnumNetworkEvents(sockArr[index],
							hEventArr[index],
							&netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:AcceptProc(index);break;
		case FD_READ:ReadProc(index);break;
		case FD_CLOSE:CloseProc(index);	break;
		}
	}
	closesocket(_socket);
}



HANDLE AddNetWorkEvent(SOCKET _socket, long _event)
{
	HANDLE hEvent = WSACreateEvent();

	sockArr[totalIndex] = _socket;
	hEventArr[totalIndex] = hEvent;	

	WSAEventSelect(_socket, hEventArr[totalIndex], _event);

	totalIndex++;

	return hEvent;
}

void AcceptProc(int _index)
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);

	memset(&clientAddr, 0, addrLen);

	SOCKET clientSocket;
	clientSocket = accept(sockArr[0], (sockaddr*)&clientAddr, &addrLen);
	if (totalIndex == WSA_MAXIMUM_WAIT_EVENTS)
	{
		printf("Serve is Full \n ip : %s \n port : %d \n user : %d fail login\n",
			inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),(int)clientSocket);
		closesocket(clientSocket);
		return;
	}
	AddNetWorkEvent(clientSocket, FD_READ | FD_CLOSE);
	printf("ip : %s \nport : %d\nuser : %d login\n",
		inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), (int)clientSocket);
}

void ReadProc(int _index)
{
	char msg[MAX_BUFFER];
	recv(sockArr[_index], msg, MAX_BUFFER, 0);

	SOCKADDR_IN clientAddr = { 0 };
	int addrLen = sizeof(clientAddr);

	getpeername(sockArr[_index], (sockaddr*)&clientAddr, &addrLen);

	char smsg[MAX_BUFFER];
	sprintf_s(smsg,"[%s : %d : %d] : %s",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),(int)sockArr[_index],msg);

	for (int i = 1; i < totalIndex; i++)
	{
		send(sockArr[i], smsg, MAX_BUFFER, 0);
	}
}

void CloseProc(int _index)
{
	closesocket(sockArr[_index]);

	WSACloseEvent(hEventArr[_index]);



	totalIndex--;

	sockArr[_index] = sockArr[totalIndex];

	hEventArr[_index] = hEventArr[totalIndex];
}

// 해결해야 할 문제들
// 
// 2클라이언트 간의 송신버퍼가
// 서로 밀리는 현상
// 서버채팅 로그 추가
// 소켓및 이벤트 배열의 꼬임 