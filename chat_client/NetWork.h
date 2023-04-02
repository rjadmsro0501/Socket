#pragma once
#include "stdafx.h"
#include "Packet.h"
#include "PacketManager.h"
#include "ChatManager.h"

class NetWork
{
public:
	NetWork();
	~NetWork();

	void Init();
	void Update();
	void Clean();

	void Set_TCPSocket();
	void Connect(short portNum);
	void Run();

	bool StartRecvThread();
	bool StartPacketProcessThread();

	SOCKET GetSocket() { return mClientSocket; }
	
	bool SendPacket(PROTOCOL protocol, char* data, DWORD dataSize, bool inGame);
private:
	WSADATA mWSAData;
	static SOCKET mClientSocket;

	static unsigned int WINAPI recvThread(LPVOID lpParam);
	static unsigned int WINAPI PacketProcessThread(LPVOID lpParam);

};

extern NetWork* g_pNetWork;
