#pragma once

#include <iostream>
#include <map>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib ,"Ws2_32.lib")
#define SERVER_PORT		3500
#define MAXBUFFER		1024	
#define MAX_ID_LEN		16
using namespace std;


// 1. 다중접속 관리
// - 클라이언트 접속시마다 id 부여
// 
// 2. 패킷 포맷 및 프로토콜 정의
// - 기본 패킷 포맷
// - 길이(byte) + 타입(type) + 데이터(Data)
// - Client -> Server
// - login요청 이동패킷
// - Server-> Client
// - login 수락 , 위치지정 , id 접속 알림, id 로그아웃 알림
// 
// 3. 패킷 처리 루틴 작성
// - bool PacketProcess(const unsigned char* pBuf, int client_id);
// - Recv의 구현 Overlapped 구조체의 확장
// 
// struct stOverEx {
//	WSAOVERLAPPED m_wsaOver;
//	WSABUF	m_wsaBuf;
//	unsigned char m_IOCPbuf[MAX_BUF_SIZE]; 	// IOCP send/recv 버퍼
//	enumOperation m_Operation;				// Send/Recv/Accept 구별
// };
// 
// - 클라이언트 정보에 추가될 내용
// class ClientInfo {
//	...
//		stOverEx	m_over;
//	unsigned char m_recv_packet_buf[MAX_PACKET_SIZE];// 패킷이 조립되는버퍼
//	int	m_prev_size;				// 이전에 받아 놓은 패킷의 양
// };
