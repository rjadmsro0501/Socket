#include <stdio.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER	1024
#define SERVER_PORT 3500
#define BLOG_SIZE	5
	
/*
* set socket opt
* 1. bind
* 2. listen
* 3. accept
* 4. recv and send
* 5. closesocket
*/ 

SOCKET SetTCPServer(short pnum, int blog);
void AcceptLoop(SOCKET sock);
void DoIt(SOCKET dosock);


SOCKET SetTCPServer(short pnum, int blog)
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// 소켓 생성
	if (sock == -1) { return -1; }

	SOCKADDR_IN serveraddr;	// 소켓 주소
	memset(&serveraddr, 0, sizeof(SOCKADDR_IN));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	int re = 0;
	re = bind(sock,(sockaddr*)&serveraddr,sizeof(serveraddr)); 
	// 소켓 주소와 네트워크 인터페이스 결합

	if (re == -1) { return -1; }

	// TCP 서버에서는 연결 요청 과정동안 
	//클라이언트 정보를 기억하기 위한 백 로그 큐를 설정

	re = listen(sock, blog); // 백 로그 큐 설정
	if (re == -1) { return -1; }
	return sock;
}

void AcceptLoop(SOCKET sock)
{
	SOCKET dosock;
	SOCKADDR_IN clientaddr;
	memset(&clientaddr, 0, sizeof(SOCKADDR_IN));
	int len = sizeof(clientaddr);
	while (true)
	{
		// accept 함수를 호출하면 클라이언트의 소켓 주소를 알수있고
		// 송수신에 사용할 소켓을 반환합니다.
		dosock = accept(sock, (SOCKADDR*)&clientaddr, &len); // 연결 수락
		if (dosock == -1)
		{
			printf("Accept fail\n");
			break;
		}

		printf("%s : %d 의 연결 요청 수락\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
		// 연결 수락 하는 accept 함수가 반환한 소켓을 이용하여 송수신 합니다
		DoIt(dosock);
	}
	closesocket(sock);
}

void DoIt(SOCKET dosock)
{	
	char msg[MAX_BUFFER] = "";
	while (recv(dosock, msg, sizeof(msg), 0) > 0) // 수신
	{
		// 에코서버에서는 메시지를 수신하는것을 반복
		// 상대가 소켓을 닫으면 recv 함수는 0을 반환
		// 그리고 수신이 실패하면 -1을 반환
		// 따라서 recv 함수가 양수를 반환하면 계속 수행
		printf("recv : %s \n", msg);
		send(dosock, msg, sizeof(msg), 0); // 송신
	}
	closesocket(dosock);
}

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2,2), &wsadata);// 윈속 초기화

	SOCKET sock = SetTCPServer(SERVER_PORT, BLOG_SIZE); // 대기 소켓 설정
	if (sock == -1)
	{
		printf("대기 소켓 오류\n");
		WSACleanup();
		return 0;
	}

	AcceptLoop(sock);
	WSACleanup();

	return 0;
}