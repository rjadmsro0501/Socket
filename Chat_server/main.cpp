#include "stdafx.h"

int g_port_number = 9999;
const int g_client_count = 10;
socketinfo sock_array[g_client_count + 1];
int g_total_socket_count = 0;


int server_init();
int server_close();
unsigned int WINAPI do_chat_service(void* _param);
unsigned int WINAPI recv_and_forward(void* _param);

int add_client(int _index);
int read_client(int _index);
void remove_client(int _index);
int notify_client(char* _msg);
char* get_client_ip(int _index);

int main()
{
	unsigned int tid;			// 스레드 id
	char msg[MAXBYTE] = "";		// 전송 버퍼
	HANDLE mainThread;			// 스레드 핸들

	mainThread = (HANDLE)_beginthreadex(NULL, 0, do_chat_service,(void*)0,0,&tid);
	// _beginthreadex(보안변수, 초기 스택사이즈, 스레드함수 모듈(반환형UINT), 스레드인자, 생성옵션, 스레드id)

	if (mainThread)
	{
		while (1)
		{
			gets_s(msg, MAXBYTE);
			if (strcmp(msg, "/x") == 0)
				break;

			notify_client(msg);
		}
		server_close();
		WSACleanup();
		CloseHandle(mainThread);
	}


	return 0;
}

int server_init()
{
	// 서버 초기화
	// WinSock 라이브러리 초기화
	// socket() 함수호출 서버소켓 생성
	// bind()
	// listen()
	WSADATA		wsaData;
	SOCKET		listenSocket;
	SOCKADDR_IN serverAddr;

	memset(&sock_array, 0, sizeof(sock_array));
	g_total_socket_count = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		puts("WSAStartup Error.");
		return -1;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (listenSocket < 0)
	{
		puts("Socket Error.");
		return -1;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(g_port_number);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		puts("bind Error.");
		return -2;
	}

	if (listen(listenSocket, SOMAXCONN) < 0)
	{
		puts("listen Error.");
		return -3;
	}

	return listenSocket;
}

int server_close()
{
	for (int i = 1; i < g_total_socket_count; i++)
	{
		closesocket(sock_array[i].m_socket);
		WSACloseEvent(sock_array[i].m_handle);
	}

	return 0;
}

unsigned int __stdcall do_chat_service(void* _param)
{
	// 채팅 서비스
	// 서버소켓 초기화
	// 클라이언트 연결 요청 수신
	// 클라이언트가 보낸 메시지를 읽고 다시전달
	SOCKET serverSocket;
	WSANETWORKEVENTS checkEventHandle;

	int eventIndex;
	WSAEVENT handleArray[g_client_count + 1];

	serverSocket = server_init();
	if (serverSocket < 0)
	{
		printf("init error.\n");
		exit(0);
	}
	else
	{
		printf("\n 서버 초기화가 완료.(port num : %d)\n", g_port_number);

		HANDLE event = WSACreateEvent();
		sock_array[g_total_socket_count].m_handle = event;
		sock_array[g_total_socket_count].m_socket = serverSocket;

		strcpy_s(sock_array[g_total_socket_count].m_nick, "sever");
		strcpy_s(sock_array[g_total_socket_count].m_ipaddr,"0.0.0.0");

		WSAEventSelect(serverSocket, event, FD_ACCEPT);
		g_total_socket_count++;

		while (true)
		{
			memset(&handleArray,0,sizeof(handleArray));
			for (int i = 0; i < g_total_socket_count; i++)
			{
				handleArray[i] = sock_array[i].m_handle;
			}

			eventIndex = WSAWaitForMultipleEvents(g_total_socket_count,
						handleArray,FALSE,WSA_INFINITE,FALSE);
			if ((eventIndex != WSA_WAIT_FAILED) && (eventIndex != WSA_WAIT_TIMEOUT))
			{
				WSAEnumNetworkEvents(sock_array[eventIndex].m_socket,
									sock_array[eventIndex].m_handle,
									&checkEventHandle);
				if (checkEventHandle.lNetworkEvents == FD_ACCEPT)
					add_client(eventIndex);
				else if (checkEventHandle.lNetworkEvents == FD_READ)
					read_client(eventIndex);
				else if (checkEventHandle.lNetworkEvents == FD_CLOSE)
					remove_client(eventIndex);
				
			}
		}
		closesocket(serverSocket);
	}
	WSACleanup();
	_endthreadex();

	return 0;
}

unsigned int __stdcall recv_and_forward(void* _param)
{
	// 클라이언트 메시지를 받아서 다른클라이언트에게 전달
	int index = (int)_param;

	char msg[MAXBYTE];
	char share_msg[MAXBYTE];

	SOCKADDR_IN clientAddr;
	int recvLen = 0;
	int addrLen = 0;

	char* token1 = NULL;
	char* next_token = NULL;

	memset(&clientAddr, 0, sizeof(clientAddr));

	if ((recvLen = recv(sock_array[index].m_socket, msg, MAXBYTE, 0)) > 0)
	{
		addrLen = sizeof(clientAddr);
		getpeername(sock_array[index].m_socket, (sockaddr*)&clientAddr, &addrLen);
		strcpy_s(share_msg, msg);
		if (strlen(sock_array[index].m_nick) <= 0)
		{
			token1 = strtok_s(msg, "]", &next_token);
			strcpy_s(sock_array[index].m_nick, token1 + 1);
		}
		for (int i = 1; i < g_total_socket_count; i++)
		{
			send(sock_array[i].m_socket,share_msg,MAXBYTE,0);
		}
	}
	_endthreadex(0);
	return 0;
}

int add_client(int _index)
{
	// 클라이언트 추가
	SOCKADDR_IN clientAddr;
	SOCKET clientSocket;
	int addrLen = sizeof(clientAddr);
	memset(&clientAddr, 0, sizeof(clientAddr));
	
	if (g_total_socket_count == FD_SETSIZE)
		return 1;
	else 
	{
		clientSocket = accept(sock_array[0].m_socket, (sockaddr*)&clientAddr, &addrLen);
		HANDLE event = WSACreateEvent();

		sock_array[g_total_socket_count].m_socket = clientSocket;
		sock_array[g_total_socket_count ].m_handle = event;
		strcpy_s(sock_array[g_total_socket_count].m_ipaddr, inet_ntoa(clientAddr.sin_addr));

		WSAEventSelect(clientSocket, event, FD_READ|FD_CLOSE);

		g_total_socket_count;
		printf("신규 클라이언트 접속 : (ip : %s) \n", inet_ntoa(clientAddr.sin_addr));

		char msg[256];
		sprintf(msg, "신규 클라이언트 접속 : (ip : %s) \n", inet_ntoa(clientAddr.sin_addr));
		
		notify_client(msg);
	}
	return 0;
}

int read_client(int _index)
{
	// 클라이언트 메시지를 읽기
	// 스레드를 사용하여 클라이언트의 메시지 수신작업을 처리
	unsigned int threadId;
	HANDLE mainThread = (HANDLE)_beginthreadex(NULL,0,recv_and_forward,(void*)_index,0,&threadId);
	WaitForSingleObject(mainThread, INFINITE);

	CloseHandle(mainThread);
	return 0;
}

void remove_client(int _index)
{
	// 접속종료한 클라이언트 정보 송신
	// 등록한 소켓 배열 삭제
	// 등록한 이벤트 객체 삭제
	// 접속한 클라이언트 인덱스 --
	// 소켓,이벤트 객체 삭제로 중간에 빈 배열 정리

	char remove_ip[256];
	char msg[MAXBYTE];

	strcpy_s(remove_ip,get_client_ip(_index));
	printf("client logout (Index : %d, IP : %s, Nick : %s)\n",_index, remove_ip, sock_array[_index].m_nick);
	sprintf_s(msg,"client logout (IP : %s , Nick : %d)\n",remove_ip,sock_array[_index].m_nick);

	closesocket(sock_array[_index].m_socket);
	WSACloseEvent(sock_array[_index].m_handle);

	g_total_socket_count--;
	sock_array[_index].m_socket = sock_array[g_total_socket_count].m_socket;
	sock_array[_index].m_handle = sock_array[g_total_socket_count].m_handle;

	strcpy_s(sock_array[_index].m_ipaddr, sock_array[g_total_socket_count].m_ipaddr);
	strcpy_s(sock_array[_index].m_nick, sock_array[g_total_socket_count].m_nick);

	notify_client(msg);
}

int notify_client(char* _msg)
{
	for (int i = 1; i < g_total_socket_count; i++)
		send(sock_array[i].m_socket, _msg, MAXBYTE, 0);
	return 0;
}

char* get_client_ip(int _index)
{
	static char ipaddress[256];
	int addr_len;
	struct sockaddr_in sock;

	addr_len = sizeof(sock);
	if (getpeername(sock_array[_index].m_socket, (struct sockaddr*)&sock, &addr_len) < 0)
		return NULL;

	strcpy_s(ipaddress, inet_ntoa(sock.sin_addr));
	return ipaddress;
}
