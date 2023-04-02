#include "PacketManager.h"

PacketManager* g_pPacketManager = nullptr;

PacketManager::PacketManager()
{
	InitializeCriticalSection(&mCS);
}

PacketManager::~PacketManager()
{
	DeleteCriticalSection(&mCS);
}

void PacketManager::Init()
{
	InitializeCriticalSection(&mCS);
}

void PacketManager::Clean()
{
	while (!mBufferQueue.empty())
	{
		mBufferQueue.pop();
	}

	DeleteCriticalSection(&mCS);
}
// recv -> enqueue -> parsing -> process

bool PacketManager::Enqueue(ClientSession* client, char* buffer)
{
	if (buffer == nullptr)
	{
		cout << "Enqueue : packet in session is null " << endl;
		return false;
	}

	ClientPacket pack(client, buffer);
	
	EnterCS();

	printf("Enqueue Packet\n");
	mBufferQueue.push(pack);

	LeaveCS();

	return true;
}

bool PacketManager::Dequeue(ClientPacket &pack)
{
	pack = mBufferQueue.front();
	mBufferQueue.pop();

	return true;
}

void PacketManager::ProcessAllQueue()
{
	while (true)
	{
		if (mStopFlag) break;

		if (!mBufferQueue.empty())
		{
			EnterCS();

			printf("Process Allqueue Run()\n");

			//printf("Packet Enqueue\n");

			ClientPacket pack = mBufferQueue.front();
			mBufferQueue.pop();

			//ClientPacket pack;
			//Dequeue(pack);

			// Packet head check
			PROTOCOL protocol = ParsingPacket(pack);

			// process packets according to the protocol
			ProcessPacket(protocol, pack);
			printf("Packet Process End\n");
			LeaveCS();
		}

		if (mStopFlag) break;
	}
}

bool PacketManager::MakeSendPacket(ClientSession* client, char* data, DWORD dataBufferSize, PROTOCOL protocol)
{
	if (client == nullptr)
	{
		cout << "PacketManager::MakeSendPacket - client is nullptr " << endl;
		return false;
	}

	// make packet
	char* p = client->GetSendOverlappedBuffer();

	// set header
	SHEAD head;
	head.mCmd = (unsigned char)protocol;
	head.mPacketSize = sizeof(SHEAD) + dataBufferSize;
	head.mTransferToInGame = false;

	memcpy(p, (char*)&head, sizeof(SHEAD));
	memcpy(p + sizeof(SHEAD), data, sizeof(dataBufferSize));
	
	// set send overlapped
	client->SetSendOverlapped();

	return true;
}

PROTOCOL PacketManager::ParsingPacket(ClientPacket packet)
{
	if (packet.mBuffer == nullptr)
	{
		cout << "parsing buffer is null, return protocol NONE" << endl;
		return PROTOCOL::NONE;
	}

	SHEAD head;
	memset(&head, 0, sizeof(SHEAD));
	memcpy(&head, packet.mBuffer, sizeof(SHEAD));

	if (head.mTransferToInGame)
	{
		cout << "Protocol : TRANFERED " << endl;
		return PROTOCOL::TRANFERED;
	}
	return (PROTOCOL)head.mCmd;
}

void PacketManager::ProcessPacket(PROTOCOL protocol, ClientPacket pack)
{
	switch (protocol)
	{
	case PROTOCOL::TRANFERED:
	{
		break;
	}
	case PROTOCOL::TEST_CHAT:
	{
		printf("Recv Protocol - TEST_CHAT\n");
		SCHAT chat;
		memset(&chat, 0, sizeof(SCHAT));
		memcpy(&chat, pack.mBuffer + sizeof(SHEAD), sizeof(SCHAT));
		cout << "Message from client :" << chat.buf << endl;
		cout << "Send to Echo Client " << endl;		
		
		MakeSendPacket(pack.mSession, (char*)&chat,sizeof(SCHAT),PROTOCOL::TEST_CHAT);
		pack.mSession->Send();	
		break;
	}
	default :
		break;
	}
}
