//192.168.35.119
#include <stdio.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER	1024
#define SERVER_PORT 3500
#define BLOG_SIZE	5
#define SERVER_IP "192.168.35.119"

int main()
{
    WSADATA wsadata;

    WSAStartup(MAKEWORD(2, 2), &wsadata);//扩加 檬扁拳    
    SOCKET sock;
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//家南 积己
    if (sock == -1) { return -1; }



    SOCKADDR_IN servaddr = { 0 };//家南 林家
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(SERVER_PORT);


    int re = 0;
    re = connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));//楷搬 夸没
    if (re == -1) { return -1; }

    char msg[MAX_BUFFER] = "";
    while (true)
    {
        gets_s(msg, MAX_BUFFER);
        send(sock, msg, sizeof(msg), 0);//价脚
        if (strcmp(msg, "exit") == 0) { break; }
        recv(sock, msg, sizeof(msg), 0);//价脚
        printf("荐脚:%s\n", msg);
    }

    closesocket(sock);//家南 摧扁
    WSACleanup();//扩加 秦力拳

    return 0;	
}