#include <stdio.h>
#include <winsock.h>

#pragma comment (lib,"Ws2_32.lib")

#define MAX_BUFFER	1024
#define SERVER_PORT 3500

int main()
{
    // Winsock Start - windock.dll �ε�
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
    {
        printf("Error - Can not load 'winsock.dll' file\n");
        return 1;
    }

    // 1. ���ϻ���    
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        printf("Error - Invalid socket\n");
        return 1;
    }

    // �������� ��ü����
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    // 2. ���ϼ���
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        printf("Error - Fail bind\n");
        // 6. ��������
        closesocket(listenSocket);
        // Winsock End
        WSACleanup();
        return 1;
    }

    // 3. ���Ŵ�⿭����
    if (listen(listenSocket, 5) == SOCKET_ERROR)
    {
        printf("Error - Fail listen\n");
        // 6. ��������
        closesocket(listenSocket);
        // Winsock End
        WSACleanup();
        return 1;
    }


    // fd_set - ���� ������ ����
    fd_set fdSet, tempFdSet;
    int fdNum;

    // fd_set �ʱ�ȭ
    // FD_ZERO(fd_set *set) fd_set�� �ʱ�ȭ
    FD_ZERO(&fdSet);    
    
    // fd_set �� listen ���� ������ȣ�� �߰�
    // FD_SET(int fd, fd_set *set) ������ ���� ��ũ���͸� fd_set�� �߰�
    FD_SET(listenSocket, &fdSet);
   
    // ������ �������� ����
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);
    memset(&clientAddr,0,addrLen);
    SOCKET clientSocket;

    while (1)
    {
        // fdSet�� ���� ���������ʰ� �����Ͽ� �����ϴ� ����
        // fd_set���� �ּҰ��� �޴´� �� ������ ���ϱ⶧����
        // select �Լ��� ȣ���ϰ� �Ǹ� ��ȭ(set)�� �ִ� ������
        // ������ ������ ���ϵ��� 0���� �ٲ����
        // ���� ȣ��� ������ �߻��ϱ⿡ ����� ��
        tempFdSet = fdSet;        
         
        // select - ��ȭ�� �ִ� ���� ����
        fdNum = select(0,&tempFdSet,NULL,NULL,NULL);

        // fd�� ������ ������ �ִ��� �˻�
        if (FD_ISSET(listenSocket, &tempFdSet))
        {
            // ������
            clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == INVALID_SOCKET)
            {
                printf("Error - Fail accept\n");
                continue;
            }

            // fd_set�� client ���� ������ȣ�� �߰�
            FD_SET(clientSocket,&fdSet);
            printf("[%d]user join \n", clientSocket);
            continue;
        }

        // ���� ������ ��ü�� Ȯ���Ͽ� ó��
        for (unsigned int i = 0; i < tempFdSet.fd_count; i++)
        {
            // listen ����(������ �����ϴ� ����) �� ó������ ����
            if (tempFdSet.fd_array[i] == listenSocket)
                continue;

            SOCKET savedSocket = tempFdSet.fd_array[i];

            // ������ �б�
            char msg[MAX_BUFFER];
            int recvByte = recv(savedSocket, msg, MAX_BUFFER, 0);
            if (recvByte > 0)
            {
                printf("[%d user]recv : %s \n",savedSocket, msg);
                send(savedSocket, msg, MAX_BUFFER, 0);
            }
            else
            {
                closesocket(clientSocket);
                FD_CLR(savedSocket, &tempFdSet);
            }
        }
    }
    closesocket(listenSocket);
    WSACleanup();
	return 0;
}