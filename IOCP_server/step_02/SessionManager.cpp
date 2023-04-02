#include "SessionManager.h"

SessionManager* g_pSessionManager = nullptr;

SessionManager::SessionManager() : mClientCount(0)
{
	InitializeCriticalSection(&CS);
}

SessionManager::~SessionManager()
{
}

ClientSession* SessionManager::CreateClientSession(SOCKET sock)
{
	ClientSession* client = new ClientSession(sock);
	
	cout << "create client session " << endl;
	mClientList.insert(ClientList::value_type(sock, client));
	
	return client;
}

void SessionManager::DeleteClientSession(ClientSession* client)
{
	mClientList.erase(client->GetSocket());

	delete client;

	cout << "client delete " << endl;
}

int SessionManager::IncreaseClientCount()
{
	EnterCriticalSection(&CS);
	mClientCount++;
	cout << "client count increase.." << endl;
	LeaveCriticalSection(&CS);

	return mClientCount;

}

int SessionManager::DecreaseClientCount()
{
	EnterCriticalSection(&CS);
	mClientCount--;
	cout << "client count decrease.." << endl;
	LeaveCriticalSection(&CS);

	return mClientCount;
}

void SessionManager::NotifyAllClient(char* _notify)
{
	map<SOCKET, ClientSession*>::iterator iter;
	
	SNOTIFY notify;
	strcpy_s(notify.buf, _notify);
	memset(&notify, 0, sizeof(SNOTIFY));
	memcpy(&notify, notify.buf + sizeof(SHEAD),sizeof(SNOTIFY));

	for (iter = mClientList.begin(); iter != mClientList.end(); iter++)
	{
		//PacketManager::GetInstance()->MakeSendPacket(iter->second,(char*)&notify,sizeof(SNOTIFY),PROTOCOL::NOTIFY);
		g_pPacketManager->MakeSendPacket(iter->second, (char*)&notify, sizeof(SNOTIFY), PROTOCOL::NOTIFY);
		iter->second->Send();
	};
}


