#include "PacketManager.h"

PacketManager* g_pPacketManager = nullptr;

PacketManager::PacketManager() : mIsStop (false)
{
	InitializeCriticalSection(&mCS);
}

PacketManager::~PacketManager()
{
	while (!mRecvBufferQueue.empty())
	{
		mRecvBufferQueue.pop();
	}
	DeleteCriticalSection(&mCS);
}

bool PacketManager::Enqueue(char* recvBuffer)
{
	if (recvBuffer == nullptr)
	{
		cout << "recvBuffer null" << endl;
		return false;
	}
	EnterCS();
	mRecvBufferQueue.push(recvBuffer);
	LeaveCS();

	return true;
}

void PacketManager::ProcessAllQueue()
{
	while (true)
	{
		if (mIsStop) break;

		if (!mRecvBufferQueue.empty())
		{
			EnterCS();
			char* buffer = mRecvBufferQueue.front();
			mRecvBufferQueue.pop();			
			LeaveCS();

			bool result = ProcessPacket(buffer);
		}

		if (mIsStop) break;
	}
}

bool PacketManager::ProcessPacket(char* recvBuffer)
{
	if (recvBuffer == nullptr)
	{
		cout << "recvBuffer is null" << endl;
		return false;
	}

	SHEAD head;
	memset(&head, 0, sizeof(SHEAD));
	memcpy(&head, recvBuffer, sizeof(SHEAD));
	PROTOCOL protocol = (PROTOCOL)head.mCmd;
	
	switch (protocol)
	{
		case PROTOCOL::TEST_CHAT:
		{
			SCHAT chat;
			memset(&chat, 0, sizeof(SCHAT));
			memcpy(&chat, recvBuffer + sizeof(SHEAD), sizeof(SCHAT));
			g_pChatManager->ShowRecvMsg(chat);
			break;
		}
		case PROTOCOL::NOTIFY :
		{
			SNOTIFY notify;
		}
		default:
			break;
	}
	
	return true;
}

int PacketManager::GetBufferQueueEmpty()
{
	if (!mRecvBufferQueue.empty())
		return true;
	else if (mRecvBufferQueue.empty())
		return false;
	
	return true;
}
