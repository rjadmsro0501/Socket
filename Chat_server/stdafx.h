#pragma once
// ���� �ʱ�ȭ �� Ŭ���̾�Ʈ ���Ӵ��
// �������� ���� Ŭ���̾�Ʈ ���� , ä�� �޽����� ���� ���� �޽�����
// �����ϱ� ���� FD_CLOSE ���
// ������ �̿��� ä�� �޽��� ���Ű� ����
// Ŭ���̾�Ʈ ����/ ���� ���� ���¿� �������� ����

// �ټ��� Ŭ���̾�Ʈ���� ���� �̺�Ʈ ���� ip �ּ� ������ ���Ե�
// sock info ����ü�� ���� �����Ǹ� sockinfo ������
// �ִ� Ŭ���̾�Ʈ ������ +1 �� ���Ѹ�ŭ���� �迭 ������ ����

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <process.h>
#include <string.h>
#include "socketInfo.h"

