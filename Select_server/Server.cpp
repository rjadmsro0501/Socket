#include <stdio.h>
#include <winsock.h>

#pragma comment (lib,"Ws2_32.lib")

#define MAX_BUFFER	1024
#define SERVER_PORT 3500

int main()
{
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


    // fd_set - 소켓 관리셋 생성
    fd_set fdSet, tempFdSet;
    int fdNum;

    // fd_set 초기화
    // FD_ZERO(fd_set *set) fd_set을 초기화
    FD_ZERO(&fdSet);    
    
    // fd_set 에 listen 소켓 지정번호를 추가
    // FD_SET(int fd, fd_set *set) 지정한 파일 디스크립터를 fd_set에 추가
    FD_SET(listenSocket, &fdSet);
   
    // 연결대기 정보변수 선언
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);
    memset(&clientAddr,0,addrLen);
    SOCKET clientSocket;

    while (1)
    {
        // fdSet을 직점 감지하지않고 복사하여 감지하는 이유
        // fd_set들은 주소값을 받는다 즉 값들이 변하기때문에
        // select 함수를 호출하게 되면 변화(set)이 있는 소켓을
        // 제외한 나머지 소켓들을 0으로 바꿔버림
        // 다음 호출시 문제가 발생하기에 백업을 함
        tempFdSet = fdSet;        
         
        // select - 변화가 있는 파일 감지
        fdNum = select(0,&tempFdSet,NULL,NULL,NULL);

        // fd에 데이터 변경이 있는지 검사
        if (FD_ISSET(listenSocket, &tempFdSet))
        {
            // 연결대기
            clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
            if (clientSocket == INVALID_SOCKET)
            {
                printf("Error - Fail accept\n");
                continue;
            }

            // fd_set에 client 소켓 지정번호를 추가
            FD_SET(clientSocket,&fdSet);
            printf("[%d]user join \n", clientSocket);
            continue;
        }

        // 소켓 관리셋 전체를 확인하여 처리
        for (unsigned int i = 0; i < tempFdSet.fd_count; i++)
        {
            // listen 소켓(연결을 관리하는 소켓) 은 처리하지 않음
            if (tempFdSet.fd_array[i] == listenSocket)
                continue;

            SOCKET savedSocket = tempFdSet.fd_array[i];

            // 데이터 읽기
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