#pragma once
#include "stdafx.h"
#include "Packet.h"
#include "PacketManager.h"
#include "ClientSession.h"

class ClientSession;
class SessionManager;

class SessionManager
{
public:
	SessionManager();
	~SessionManager();

	ClientSession* CreateClientSession(SOCKET sock);

	void DeleteClientSession(ClientSession* client);
	int IncreaseClientCount();
	int DecreaseClientCount();

	int GetClientCount() { return mClientCount; }
	// GetClient() {return mClientList.begin() };

	void NotifyAllClient(char* _notify);
private:
	typedef map<SOCKET, ClientSession*>ClientList; 
	ClientList mClientList;
	int mClientCount;
	CRITICAL_SECTION CS;
};

extern SessionManager* g_pSessionManager;

