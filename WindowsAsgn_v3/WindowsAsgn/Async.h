#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

#define PORT 5150
#define DATA_BUFSIZE 8192
#define ACK  0x06
#define SOT  0x02
#define EOT  0x04

typedef struct _SOCKET_INFORMATION {
	BOOL RecvPosted;
	CHAR Buffer[DATA_BUFSIZE];
	OVERLAPPED Overlapped;
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSEND;
	DWORD BytesRECV;
	_SOCKET_INFORMATION *Next;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;
