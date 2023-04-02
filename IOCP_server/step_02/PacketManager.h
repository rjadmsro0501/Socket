#pragma once
#include <queue>
#include "stdafx.h"
#include "Packet.h"
#include "ClientSession.h"
#include "SessionManager.h"
#include "TemplateSingleton.h"

struct ClientPacket
{
	/*ClientPacket() : mSession(nullptr)
	{
		memset(&mBuffer, 0, MAXBUFFER);
	}
	ClientPacket(ClientSession* client, char* buffer) : mSession(client)
	{
		memset(&mBuffer, 0, MAXBUFFER);
		memcpy(&mBuffer, buffer, sizeof(buffer));
	}*/
	ClientPacket(ClientSession* client, char* buffer) : mSession(client), mBuffer(buffer) {}

	ClientSession* mSession;
	char *mBuffer;
};

class PacketManager //: public TemplateSingleton<PacketManager>
{
public:
	PacketManager();
	~PacketManager();

	void Init();
	void Clean();

	bool Enqueue(ClientSession* client, char* buffer);
	bool Dequeue(ClientPacket &pack);

	void ProcessAllQueue();

	bool MakeSendPacket(ClientSession* client, char* buffer, DWORD dataBufferSize, PROTOCOL protocol);

	PROTOCOL ParsingPacket(ClientPacket packet);
	void ProcessPacket(PROTOCOL protocol, ClientPacket pack);

	void EnterCS() { EnterCriticalSection(&mCS); }
	void LeaveCS() { LeaveCriticalSection(&mCS); }

	void SetStopFlag(bool stopFlag) { mStopFlag = stopFlag; }
private:
	queue<ClientPacket> mBufferQueue;

	CRITICAL_SECTION mCS;

	bool mStopFlag = false;
};

extern PacketManager* g_pPacketManager;