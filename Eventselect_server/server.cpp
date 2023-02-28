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
    // 1. 소켓생성 > 2. 소켓설정 > 3. 수신대기열생성
    // Winsock Start - windock.dll 로드
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
    {
        printf("Error - Can not load 'winsock.dll' file\n");
        return 1;
    }

    // 1. 소켓생성  
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        printf("Error - Invalid socket\n");
        return 1;
    }

    // 서버정보 객체설정
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    // 2. 소켓설정
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        printf("Error - Fail bind\n");
        // 6. 소켓종료
        closesocket(listenSocket);
        // Winsock End
        WSACleanup();
        return 1;
    }

    // 3. 수신대기열생성
    if (listen(listenSocket, 5) == SOCKET_ERROR)
    {
        printf("Error - Fail listen\n");
        // 6. 소켓종료
        closesocket(listenSocket);
        // Winsock End
        WSACleanup();
        return 1;
    }

    // Create Event 최대 갯수의 소켓과 이벤트 객체 생성
    struct SOCKETINFO* sockets[WSA_MAXIMUM_WAIT_EVENTS];
    WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
    int totalIndex = 0;

    // listen socket 설정
    struct SOCKETINFO* socketInfo;
    socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
    memset((struct SOCKETINFO*)socketInfo, 0x00, sizeof(struct SOCKETINFO));

    socketInfo->socket = listenSocket;
    socketInfo->receiveBytes = 0;
    socketInfo->sendBytes = 0;

    sockets[totalIndex] = socketInfo;
    // 동적 할당된 생성된 socketInfo 구조체를 sockets 배열에 등록
    // totalIndex 변수는 등록된 소켓의 개수를 나타냄
    // 새로운 소켓이 등록될때 마다 1씩 증가
    
    // 이벤트 객체 생성
    events[totalIndex] = WSACreateEvent();
    // 이벤트 객체는 비동기 IO모델에서 이벤트를 통해
    // IO 작업의 완료 여부를 식별하고 처리하는데 사용
    // 객체의 핸들인 WSAEVENT 자료형의 값을 반환
    // 호출에 실패하면 WSA_INVALI_EVENT 값이 반환
    
    // WSACreateEvent() 함수를 사용하여 생성된 객체는
    // WSAEventSelect() 함수와 함계 사용됨
    // WSAEventSelect() 함수를 호출하면 소켓에서 발생하는
    // 이벤트를 해당 이벤트 객체와 연결할 수 있다. 이렇게 연결된 이벤트객체는 
    // WSAWaitForMultipleEvents() 함수를 사용하여 이벤트 핸들링을 대기
    // 소켓 이벤트가 발생했을때 해당 소켓에서 호출
    // 이벤트 객체의 사용이 끝났을때는 WSACloseEvent() 함수를 호출하여 객체를 닫음

    if (events[totalIndex] == WSA_INVALID_EVENT)
    {
        printf("Error Event Create Fail\n");
        return 1;
    }

    // 이벤트 객체를 소켓과 묶음
    if (WSAEventSelect(listenSocket, events[totalIndex], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
    {
        printf("Error Event Select Fail\n");
        return 1;
    }
    // WSAEventSelect() 함수는 소켓의 이벤트를 등록하는 함수
    // WSAWaitForMultipleEvents() 함수를 사용하여 모니터링
    // WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);
    // s : 이벤트를 등록할 소켓의 핸들을 나타내는 매개변수
    // hEventObject : 이벤트 핸들을 나타내는 매개변수
    // lNetworkEvents : 등록할 이벤트 유형을 나타내는 매개변수
    // lNetworkEvents 매개변수는 
    // FD_READ, FD_WRITE, FD_OOB, FD_ACCEPT, FD_CONNECT, FD_CLOSE
    // 상수중 하나이상의 비트OR 연산으로 조합하여 사용가능
    // 이를 통해 소켓에서 발생한 특정 이벤트를 감지
    // 등록 성공시 0 반환 실패시 소켓오류 코드 반환

    totalIndex = totalIndex + 1;

    WSANETWORKEVENTS networkEvents;
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);

    memset(&clientAddr, 0, addrLen);
    SOCKET clientSocket;
    char msg[MAX_BUFFER];

    while (1)
    {
        // 이벤트를 기다림
        int eventIndex = WSAWaitForMultipleEvents(totalIndex, events, FALSE, WSA_INFINITE, FALSE);
        
        // WSAWaitForMultipleEvents() 함수는 이벤트 객체들의 상태를 확인하고
        // 이벤트가 발생할때까지 대기하는 함수
        // DWORD WSAWaitForMultipleEvents
        //(DWORD cEvents,  대기할 이벤트의 개수
        // const WSAEVENT *lphEvents,  이벤트 객체 핸들들의 배열
        // BOOL fWaitAll, 이벤트중 하나 이상이 발생하기를 기다릴지 모든 이벤트가 발생하기를 기다릴지 정하는 부울값
        // DWORD dwTimeout, 대기시간을 지정 
        // BOOL fAlertable); 스레드 알림을 사용할지 나타내는 부울값
        // DWORD 를 반환 이 값은 대기한 이벤트중 발생한 첫 번째 이벤트의
        // 인덱스를 나타냄

        // 예외처리
        if (eventIndex == WSA_WAIT_FAILED)
        {
            printf("Error Event Wait Fail\n");
            break;
        }

        // 신호상태인 이벤트 객체의 정보를 가져옴
        if (WSAEnumNetworkEvents(
            sockets[eventIndex - WSA_WAIT_EVENT_0]->socket,
            events[eventIndex - WSA_WAIT_EVENT_0], &networkEvents) == SOCKET_ERROR)
        {
            printf("Error Event Type Error\n");
            break;
        }

        // FD_ACCEPT : 클라이언트 요청이벤트일 경우 
        // accept 함수로 소켓을 만들고 이벤트 객체와 묶음
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

            // 연결대기
            clientSocket = accept(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket,
                                 (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == SOCKET_ERROR)
            {
                printf("Error Fail accept\n");
                continue;
            }
            printf("%s : %d 의 연결 요청 수락\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            // 소켓 생성
            sockets[totalIndex] = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
            memset((struct SOCKETINFO*)sockets[totalIndex],0x00,sizeof(struct SOCKETINFO));
            sockets[totalIndex]->socket = clientSocket;
            sockets[totalIndex]->receiveBytes = 0;
            sockets[totalIndex]->sendBytes = 0;

            // 이벤트 객체 생성
            events[totalIndex] = WSACreateEvent();
            if (events[totalIndex] == WSA_INVALID_EVENT)
            {
                printf("Error Event Create Fail\n");
                return 1;
            }

            // 이벤트 객체를 소켓과 묶음
            if (WSAEventSelect(clientSocket, events[totalIndex], FD_READ) == SOCKET_ERROR)
            {
                printf("Error Event Select Fail\n");
                return 1;
            }
            totalIndex = totalIndex + 1;
        }
        // FD_READ 읽기 이벤트 일경우
        if (networkEvents.lNetworkEvents & FD_READ)
        {
            // 데이터 읽기
            int receiveBytes = recv(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket,msg, MAX_BUFFER,0);
            if (receiveBytes > 0)
            {
                printf("[% user]recv : %s \n", (int)sockets[eventIndex - WSA_WAIT_EVENT_0]->socket, msg);
                // 데이터 쓰기
                int sendBytes = send(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket, msg, strlen(msg), 0);
                if (sendBytes > 0)
                {
                    printf("[%d user]Send : %s \n", (int)sockets[eventIndex - WSA_WAIT_EVENT_0]->socket, msg);
                }
            }
        }

        // FD_CLOSE 종료 이벤트일경우
        if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            // 클라이언트 소켓종료
            closesocket(sockets[eventIndex - WSA_WAIT_EVENT_0]->socket);

            // 메모리 해제
            free((void*)sockets[eventIndex - WSA_WAIT_EVENT_0]);

            // 이벤트 객체 닫기
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