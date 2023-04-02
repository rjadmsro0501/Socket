#pragma once
#include "stdafx.h"
#include "NetWork.h"
#include "Packet.h"
#include "PacketManager.h"
#include "TemplateSingleton.h"


class ChatManager
{
public:
	void Init();
	void ChatService();
	void ShowRecvMsg(SCHAT _schat);
	void exit();

private:
	char mSendMsg[512];
	char mRecvMsg[512];
	char *input;
	char *mExit;
};

extern ChatManager* g_pChatManager;