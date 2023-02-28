#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
//#include <winsock.h>
#include <winsock2.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")


#define MAX_BUFFER      1024
#define SERVER_PORT     3500

struct SOCKETINFO
{
    SOCKET socket;
    char messageBuffer[MAX_BUFFER];
    int receiveBytes;
    int sendBytes;
};

int main()
{
    // 1. ���ϻ��� > 2. ���ϼ��� > 3. ���Ŵ�⿭����
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

    // Create Event �ִ� ������ ���ϰ� �̺�Ʈ ��ü ����
    struct SOCKETINFO* sockets[WSA_MAXIMUM_WAIT_EVENTS];
    WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
    int totalIndex = 0;

    // listen socket ����
    struct SOCKETINFO* socketInfo;
    socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
    memset((struct SOCKETINFO*)socketInfo, 0x00, sizeof(struct SOCKETINFO));

    socketInfo->socket = listenSocket;
    socketInfo->receiveBytes = 0;
    socketInfo->sendBytes = 0;

    sockets[totalIndex] = socketInfo;
    // ���� �Ҵ�� ������ socketInfo ����ü�� sockets �迭�� ���
    // totalIndex ������ ��ϵ� ������ ������ ��Ÿ��
    // ���ο� ������ ��ϵɶ� ���� 1�� ����
    
    // �̺�Ʈ ��ü ����
    events[totalIndex] = WSACreateEvent();
    // �̺�Ʈ ��ü�� �񵿱� IO�𵨿��� �̺�Ʈ�� ����
    // IO �۾��� �Ϸ� ���θ� �ĺ��ϰ� ó���ϴµ� ���
    // ��ü�� �ڵ��� WSAEVENT �ڷ����� ���� ��ȯ
    // ȣ�⿡ �����ϸ� WSA_INVALI_EVENT ���� ��ȯ
    
    // WSACreateEvent() �Լ��� ����Ͽ� ������ ��ü��
    // WSAEventSelect() �Լ��� �԰� ����
    // WSAEventSelect() �Լ��� ȣ���ϸ� ���Ͽ��� �߻��ϴ�
    // �̺�Ʈ�� �ش� �̺�Ʈ ��ü�� ������ �� �ִ�. �̷��� ����� �̺�Ʈ��ü�� 
    // WSAWaitForMultipleEvents() �Լ��� ����Ͽ� �̺�Ʈ �ڵ鸵�� ���
    // ���� �̺�Ʈ�� �߻������� �ش� ���Ͽ��� ȣ��
    // �̺�Ʈ ��ü�� ����� ���������� WSACloseEvent() �Լ��� ȣ���Ͽ� ��ü�� ����

    if (events[totalIndex] == WSA_INVALID_EVENT)
    {
        printf("Error Event Create Fail\n");
        return 1;
    }

    // �̺�Ʈ ��ü�� ���ϰ� ����
    if (WSAEventSelect(listenSocket, events[totalIndex], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
    {
        printf("Error Event Select Fail\n");
        return 1;
    }
    // WSAEventSelect() �Լ��� ������ �̺�Ʈ�� ����ϴ� �Լ�
    // WSAWaitForMultipleEvents() �Լ��� ����Ͽ� ����͸�
    // WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);
    // s : �̺�Ʈ�� ����� ������ �ڵ��� ��Ÿ���� �Ű�����
    // hEventObject : �̺�Ʈ �ڵ��� ��Ÿ���� �Ű�����
    // lNetworkEvents : ����� �̺�Ʈ ������ ��Ÿ���� �Ű�����
    // lNetworkEvents �Ű������� 
    // FD_READ, FD_WRITE, FD_OOB, FD_ACCEPT, FD_CONNECT, FD_CLOSE
    // ����� �ϳ��̻��� ��ƮOR �������� �����Ͽ� ��밡��
    // �̸� ���� ���Ͽ��� �߻��� Ư�� �̺�Ʈ�� ����
    // ��� ������ 0 ��ȯ ���н� ���Ͽ��� �ڵ� ��ȯ

    totalIndex = totalIndex + 1;

    WSANETWORKEVENTS networkEvents;
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);

    memset(&clientAddr, 0, addrLen);
    SOCKET clientSocket;
    char msg[MAX_BUFFER];

    while (1)
    {
        // �̺�Ʈ�� ��ٸ�
        int eventIndex = WSAWaitForMultipleEvents(totalIndex, events, FALSE, WSA_INFINITE, FALSE);
        
        // WSAWaitForMultipleEvents() �Լ��� �̺�Ʈ ��ü���� ���¸� Ȯ���ϰ�
        // �̺�Ʈ�� �߻��Ҷ����� ����ϴ� �Լ�
        // DWORD WSAWaitForMultipleEvents
        //(DWORD cEvents,  ����� �̺�Ʈ�� ����
        // const WSAEVENT *lphEvents,  �̺�Ʈ ��ü �ڵ���� �迭
        // BOOL fWaitAll, �̺�Ʈ�� �ϳ� �̻��� �߻��ϱ⸦ ��ٸ��� ��� �̺�Ʈ�� �߻��ϱ⸦ ��ٸ��� ���ϴ� �οﰪ
        // DWORD dwTimeout, ���ð��� ���� 
        // BOOL fAlertable); ������ �˸��� ������� ��Ÿ���� �οﰪ
        // DWORD �� ��ȯ �� ���� ����� �̺�Ʈ�� �߻��� ù ��° �̺�Ʈ��
        // �ε����� ��Ÿ��

        // ����ó��
        if (eventIndex == WSA_WAIT_FAILED)
        {
            printf("Error Event Wait Fail\n");
            break;
        }

        // ��ȣ������ �̺�Ʈ ��ü�� ������ ������
        if (WSAEnumNetworkEvents(
            sockets[eventIndex - WSA_WAIT_EVENT_0]->socket,
            events[eventIndex - WSA_WAIT_EVENT_0], &networkEvents) == SOCKET_ERROR)
        {
            printf("Error Event Type Error\n");
            break;
        }

        // FD_ACCEPT : Ŭ���̾�Ʈ ��û�̺�Ʈ�� ��� 
        // accept �Լ��� ������ ����� �̺�Ʈ ��ü�� ����
        if (networkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                printf("Error Network Error\n");
                break;
            }
            if (totalIndex > WSA_MAXIMUM_WAIT_EVENTS)
            {
                printf("Error Connection Full\n");
                break;
            }

            // ������
            clientSocket = accept(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket,
                                 (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == SOCKET_ERROR)
            {
                printf("Error Fail accept\n");
                continue;
            }
            printf("%s : %d �� ���� ��û ����\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            // ���� ����
            sockets[totalIndex] = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
            memset((struct SOCKETINFO*)sockets[totalIndex],0x00,sizeof(struct SOCKETINFO));
            sockets[totalIndex]->socket = clientSocket;
            sockets[totalIndex]->receiveBytes = 0;
            sockets[totalIndex]->sendBytes = 0;

            // �̺�Ʈ ��ü ����
            events[totalIndex] = WSACreateEvent();
            if (events[totalIndex] == WSA_INVALID_EVENT)
            {
                printf("Error Event Create Fail\n");
                return 1;
            }

            // �̺�Ʈ ��ü�� ���ϰ� ����
            if (WSAEventSelect(clientSocket, events[totalIndex], FD_READ) == SOCKET_ERROR)
            {
                printf("Error Event Select Fail\n");
                return 1;
            }
            totalIndex = totalIndex + 1;
        }
        // FD_READ �б� �̺�Ʈ �ϰ��
        if (networkEvents.lNetworkEvents & FD_READ)
        {
            // ������ �б�
            int receiveBytes = recv(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket,msg, MAX_BUFFER,0);
            if (receiveBytes > 0)
            {
                printf("[% user]recv : %s \n", (int)sockets[eventIndex - WSA_WAIT_EVENT_0]->socket, msg);
                // ������ ����
                int sendBytes = send(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket, msg, strlen(msg), 0);
                if (sendBytes > 0)
                {
                    printf("[%d user]Send : %s \n", (int)sockets[eventIndex - WSA_WAIT_EVENT_0]->socket, msg);
                }
            }
        }

        // FD_CLOSE ���� �̺�Ʈ�ϰ��
        if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            // Ŭ���̾�Ʈ ��������
            closesocket(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket);

            // �޸� ����
            free((void*)sockets[eventIndex - WSA_WAIT_EVENT_0]);

            // �̺�Ʈ ��ü �ݱ�
            if (WSACloseEvent(events[eventIndex]) == TRUE)
            {
                printf("Error Event Close Success\n");
            }
            else
            {
                printf("Error Event Close Fail\n");

            }
            for (int i = eventIndex; i < totalIndex; i++)
            {
                sockets[i] = sockets[i + 1];
                events[i] = events[i + 1];
            }
            totalIndex--;
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}