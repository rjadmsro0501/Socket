#include "ChatManager.h"

ChatManager* g_pChatManager = nullptr;

void ChatManager::Init()
{
	//msg[MAXBUFFER] = { 0 };
	
	//mExit[10] = {};

	ChatService();
}

void ChatManager::ChatService()
{
	while (true)
	{
		input = new char[256];

		if (g_pPacketManager->GetBufferQueueEmpty() == false)
		{
			printf("Input message :");
			gets_s(input, sizeof(input));
		}
		
	
		printf("Send message : %s \n", input);
		memcpy(mSendMsg, input, sizeof(input));

		//sprintf_s(mSendMsg,sizeof(mSendMsg), "Send message : %s", input);
		g_pNetWork->SendPacket(PROTOCOL::TEST_CHAT, (char*)&mSendMsg, sizeof(mSendMsg), false);		

		mExit = strrchr(mSendMsg, '/');
		if (mExit)
		{
			if (strcmp(mExit, "/exit") == 0)
				break;
		}

		memset(mSendMsg, 0, sizeof(mSendMsg));
		delete[] input;
	}
}

void ChatManager::ShowRecvMsg(SCHAT _schat)
{
	printf("Recv message : %s \n", _schat.buf);
}

void ChatManager::exit()
{

}
