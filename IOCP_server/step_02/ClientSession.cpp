#include "ClientSession.h"
#include "IocpManager.h"
#include "SessionManager.h"

ClientSession::ClientSession(SOCKET sock) : mIsConnected (false), mSocket(sock), mIsLogin(false)
{
	memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));
	memset(&mRecvOverlapped, 0, sizeof(SOVERLAPPED));
	memset(&mSendOverlapped, 0, sizeof(SOVERLAPPED));

	memset(mID, 0, MAX_ID_LEN);

}

ClientSession::~ClientSession()
{
}

bool ClientSession::OnConnect(SOCKADDR_IN* addr)
{
	// setting socket io mode - not 0 : nonblocking mode / 0 : blocking mode
	u_long arg = 1;
	ioctlsocket(mSocket, FIONBIO, &arg);

	// setting socket option
	int opt = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int));
	
	opt = 0; 
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
	{
		cout << "SO_RCVBUF change error : " << GetLastError() << endl;
		return false;
	}

	HANDLE hCP = CreateIoCompletionPort((HANDLE)mSocket, g_pIocpManager->GetCP(), (ULONG_PTR)this, 0);
			
	// except error for cp handle
	if (hCP != g_pIocpManager->GetCP())
	{
		cout << "CreateIoCompletionPort Error : "<<GetLastError() << endl;
		return false;
	}

	memcpy(&mClientAddr, addr, sizeof(SOCKADDR_IN));

	mIsConnected = true;

	cout << "Client Connected : IP = " << inet_ntoa(mClientAddr.sin_addr) << "PORT = " << ntohs(mClientAddr.sin_port) << endl;
	
	g_pSessionManager->IncreaseClientCount();

	return true;
}

bool ClientSession::DisConnect()
{
	if (!IsConnected())
		return false;

	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)))
	{
		cout << "linger option error..." << endl;
	}

	cout << " Client Disconnected: IP = " << inet_ntoa(mClientAddr.sin_addr) << " PORT = " << ntohs(mClientAddr.sin_port) << endl;
	
	g_pSessionManager->DecreaseClientCount();

	closesocket(mSocket);

	mIsConnected = false;
	return true;
}

bool ClientSession::IsConnected() const
{
	return mIsConnected;
}

bool ClientSession::Recv()
{
	//except error
	if (!IsConnected())
		return false;

	printf("Enter Recv()\n");
	//printf("Recv() Run\n");

	DWORD flags = 0;
	DWORD recvBytes = 0;

	mRecvOverlapped.mIOType = IOTYPE::IO_RECV;
	mRecvOverlapped.mWSABuf.buf = mRecvOverlapped.mBuffer;
	mRecvOverlapped.mWSABuf.len = MAXBUFFER;

	if (SOCKET_ERROR == WSARecv(mSocket, &mRecvOverlapped.mWSABuf, 1, &recvBytes, &flags, (LPWSAOVERLAPPED)&mRecvOverlapped, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "recv error :" << WSAGetLastError() << endl;
			return false;
		}
	}
	else
	{
		printf("Recv() Run\n");
	}

	return true;
}

bool ClientSession::Send()
{
	// excpet error
	if (!IsConnected())
		return false;
	
	DWORD flags = 0;
	DWORD sendBytes = 0;
	mSendOverlapped.mIOType = IOTYPE::IO_SEND;

	if (WSASend(mSocket, &mSendOverlapped.mWSABuf, 1, &sendBytes, flags, (LPWSAOVERLAPPED)&mSendOverlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "send error " << endl;
			return false;
		}
	}
	return true;
}

bool ClientSession::SetSendOverlapped(char* buffer, int bufferSize)
{
	if (buffer == nullptr)
		return false;

	memcpy(&mSendOverlapped.mBuffer, buffer, bufferSize);
	mSendOverlapped.mWSABuf.buf = mSendOverlapped.mBuffer;
	mSendOverlapped.mWSABuf.len = MAXBUFFER;
	return true;
}

bool ClientSession::SetSendOverlapped()
{
	mSendOverlapped.mWSABuf.buf = mSendOverlapped.mBuffer;
	mSendOverlapped.mWSABuf.len = MAXBUFFER;
	return true;
}
