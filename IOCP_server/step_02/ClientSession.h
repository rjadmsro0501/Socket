#pragma once
#include "stdafx.h"
#include "Packet.h"

class SessionManager;

class ClientSession
{
public:
	ClientSession(SOCKET sock);
	~ClientSession();

	bool OnConnect(SOCKADDR_IN* addr);
	bool DisConnect();

	bool IsConnected() const;
	bool Recv();
	bool Send();

	// get
	SOCKET GetSocket() { return mSocket; }
	
	SOVERLAPPED GetRecvOverlapped() { return mRecvOverlapped; }
	SOVERLAPPED GetSendOverlapped() { return mSendOverlapped; }

	char* GetSendOverlappedBuffer() { return mSendOverlapped.mBuffer; }

	// set
	void SetPlayerIndex(int playerIndex) { mPlayerIndex = playerIndex; }

	bool SetSendOverlapped();
	bool SetSendOverlapped(char* buffer, int bufferSize);

	// id
	void	SetID(char* _id)	{ strcpy_s(mID, _id); }
	char*	GetID()				{ return mID; }
	void	CleanID()			{ memset(mID, 0, MAX_ID_LEN); }

	// login
	void SetIsLogin(bool _isLogin)	{ mIsLogin = _isLogin; }
	bool GetIsLogin()				{ return mIsLogin; }
private:
	bool mIsConnected;
	SOCKET mSocket;

	SOCKADDR_IN mClientAddr;

	SOVERLAPPED mRecvOverlapped;
	SOVERLAPPED mSendOverlapped;

	int mPlayerIndex;

	char mID[MAX_ID_LEN];
	bool mIsLogin;
};

