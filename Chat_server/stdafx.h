#pragma once
// 소켓 초기화 및 클라이언트 접속대기
// 소켓으로 부터 클라이언트 접속 , 채팅 메시지와 서비스 종료 메시지를
// 수신하기 위해 FD_CLOSE 등록
// 소켓을 이용한 채팅 메시지 수신과 전달
// 클라이언트 연결/ 접속 종료 상태와 공지사항 전달

// 다수의 클라이언트들은 소켓 이벤트 별명 ip 주소 정보가 포함된
// sock info 구조체를 통해 관리되며 sockinfo 변수는
// 최대 클라이언트 개수에 +1 을 더한만큼으로 배열 변수로 선언

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <process.h>
#include <string.h>
#include "socketInfo.h"

