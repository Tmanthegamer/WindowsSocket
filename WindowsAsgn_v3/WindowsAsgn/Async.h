#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#define PORT 5150
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
	BOOL RecvPosted;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSEND;
	DWORD BytesRECV;
	_SOCKET_INFORMATION *Next;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

DWORD WINAPI ProcessIO(LPVOID lpParameter);

BOOL OpenPortForSending(HANDLE file, int packet_size, int frequency, char* protocol);

BOOL IncomingConnections(SOCKET ListenSocket);