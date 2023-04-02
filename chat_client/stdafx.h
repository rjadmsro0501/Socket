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

#pragma comment(lib ,"Ws2_32.lib")
#define SERVER_PORT		3500
#define MAXBUFFER		1024	
#define MAX_ID_LEN		16
//#define SERVER_IP		"192.168.35.119"
#define SERVER_IP		"127.0.0.1"
using namespace std;
