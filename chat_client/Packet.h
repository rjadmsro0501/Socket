#pragma once

#include "stdafx.h"

enum class IOOperation
{
	RECV,
	SEND
};

enum class IOTYPE
{
	IO_NONE,
	IO_SEND,
	IO_RECV,
	IO_RECV_ZERO,
	IO_ACCEPT,
	IO_DISCONNECT
};

enum class PROTOCOL
{
	NONE,

	TRANFERED,

	TEST_CHAT,

	NOTIFY
};

typedef struct sPacket_Head
{
	sPacket_Head() : mTransferToInGame(false) {}

	UCHAR mCmd;
	DWORD mPacketSize;
	bool mTransferToInGame;
} SHEAD;

typedef struct sPacket_Tail
{
	bool end;
} STAIL;

struct SCHAT
{
	char buf[MAXBUFFER];
};

struct SNOTIFY
{
	char buf[MAXBUFFER];
};

typedef struct stOverlappedEx
{
	stOverlappedEx()
	{
		// init member
		memset(mBuffer, 0, MAXBUFFER);
		memset(&mWSABuf, 0, sizeof(WSABUF));
	}
	WSAOVERLAPPED	mWSAOverlapped;
	IOTYPE			mIOType;
	WSABUF			mWSABuf;
	char			mBuffer[MAXBUFFER];
	//SOCKET			mClientSocket;
	//IOOperation		meOperation;
}SOVERLAPPED;
