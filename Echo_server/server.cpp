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
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// ���� ����
	if (sock == -1) { return -1; }

	SOCKADDR_IN serveraddr;	// ���� �ּ�
	memset(&serveraddr, 0, sizeof(SOCKADDR_IN));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	int re = 0;
	re = bind(sock,(sockaddr*)&serveraddr,sizeof(serveraddr)); 
	// ���� �ּҿ� ��Ʈ��ũ �������̽� ����

	if (re == -1) { return -1; }

	// TCP ���������� ���� ��û �������� 
	//Ŭ���̾�Ʈ ������ ����ϱ� ���� �� �α� ť�� ����

	re = listen(sock, blog); // �� �α� ť ����
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
		// accept �Լ��� ȣ���ϸ� Ŭ���̾�Ʈ�� ���� �ּҸ� �˼��ְ�
		// �ۼ��ſ� ����� ������ ��ȯ�մϴ�.
		dosock = accept(sock, (SOCKADDR*)&clientaddr, &len); // ���� ����
		if (dosock == -1)
		{
			printf("Accept fail\n");
			break;
		}

		printf("%s : %d �� ���� ��û ����\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
		// ���� ���� �ϴ� accept �Լ��� ��ȯ�� ������ �̿��Ͽ� �ۼ��� �մϴ�
		DoIt(dosock);
	}
	closesocket(sock);
}

void DoIt(SOCKET dosock)
{	
	char msg[MAX_BUFFER] = "";
	while (recv(dosock, msg, sizeof(msg), 0) > 0) // ����
	{
		// ���ڼ��������� �޽����� �����ϴ°��� �ݺ�
		// ��밡 ������ ������ recv �Լ��� 0�� ��ȯ
		// �׸��� ������ �����ϸ� -1�� ��ȯ
		// ���� recv �Լ��� ����� ��ȯ�ϸ� ��� ����
		printf("recv : %s \n", msg);
		send(dosock, msg, sizeof(msg), 0); // �۽�
	}
	closesocket(dosock);
}

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2,2), &wsadata);// ���� �ʱ�ȭ

	SOCKET sock = SetTCPServer(SERVER_PORT, BLOG_SIZE); // ��� ���� ����
	if (sock == -1)
	{
		printf("��� ���� ����\n");
		WSACleanup();
		return 0;
	}

	AcceptLoop(sock);
	WSACleanup();

	return 0;
}