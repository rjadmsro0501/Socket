#pragma once
#include "NetWork.h"
#include "ChatManager.h"
#include "PacketManager.h"

int main()
{
	g_pNetWork = new NetWork;
	g_pPacketManager = new PacketManager;
	g_pChatManager = new ChatManager;

	//g_pNetWork->Init();
	g_pNetWork->Run();
	g_pChatManager->Init();


	delete g_pChatManager;

	g_pPacketManager->SetStopFlag(true);

	delete g_pPacketManager;
	delete g_pNetWork;
	return 0;
}