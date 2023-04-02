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


// 1. �������� ����
// - Ŭ���̾�Ʈ ���ӽø��� id �ο�
// 
// 2. ��Ŷ ���� �� �������� ����
// - �⺻ ��Ŷ ����
// - ����(byte) + Ÿ��(type) + ������(Data)
// - Client -> Server
// - login��û �̵���Ŷ
// - Server-> Client
// - login ���� , ��ġ���� , id ���� �˸�, id �α׾ƿ� �˸�
// 
// 3. ��Ŷ ó�� ��ƾ �ۼ�
// - bool PacketProcess(const unsigned char* pBuf, int client_id);
// - Recv�� ���� Overlapped ����ü�� Ȯ��
// 
// struct stOverEx {
//	WSAOVERLAPPED m_wsaOver;
//	WSABUF	m_wsaBuf;
//	unsigned char m_IOCPbuf[MAX_BUF_SIZE]; 	// IOCP send/recv ����
//	enumOperation m_Operation;				// Send/Recv/Accept ����
// };
// 
// - Ŭ���̾�Ʈ ������ �߰��� ����
// class ClientInfo {
//	...
//		stOverEx	m_over;
//	unsigned char m_recv_packet_buf[MAX_PACKET_SIZE];// ��Ŷ�� �����Ǵ¹���
//	int	m_prev_size;				// ������ �޾� ���� ��Ŷ�� ��
// };
