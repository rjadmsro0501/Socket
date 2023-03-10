#pragma once
#include "stdafx.h"

typedef struct socketinfo
{
	SOCKET m_socket;
	HANDLE m_handle;

	char m_nick[50];
	char m_ipaddr[50];

}sockinfo_t;

