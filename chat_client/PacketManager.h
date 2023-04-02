#pragma once
#include <queue>
#include "NetWork.h"
#include "Packet.h"
#include "stdafx.h"
#include "TemplateSingleton.h"
#include "ChatManager.h"

class PacketManager
{
public :
	PacketManager();
	~PacketManager();

	bool Enqueue(char* recvBuffer);

	void ProcessAllQueue();

	bool ProcessPacket(char* recvBuffer);

	void EnterCS() { EnterCriticalSection(&mCS); }
	void LeaveCS() { LeaveCriticalSection(&mCS); }

	void SetStopFlag(bool isStop) { mIsStop = isStop; }

	int GetBufferQueueEmpty();
private :

	queue<char*>mRecvBufferQueue;
	CRITICAL_SECTION mCS;
	bool mIsStop;
};

extern PacketManager* g_pPacketManager;
