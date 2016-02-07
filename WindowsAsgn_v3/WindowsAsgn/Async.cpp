// Module Name: overlap.cpp
//
// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the Overlapped I/O model with event notification. This 
//    sample is implemented as a console-style application and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts them
//    as they arrive. When this application receives data from a client, it
//    simply echos (this is why we call it an echo server) the data back in
//    it's original form until the client closes the connection.
//
// Compile:
//
//    cl -o overlap overlap.cpp ws2_32.lib
//
// Command Line Options:
//
//    overlap.exe 
//
//    Note: There are no command line options for this sample.
#include "Async.h"
/*
DWORD WINAPI ProcessIO(LPVOID lpParameter);

DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION CriticalSection;

char* sbuf;
int length;

BOOL fin = FALSE;

typedef struct {
	int packet_size;
	HANDLE file;
	int times;
} Container;

BOOL OpenPortForSending(HANDLE file, int packet_size, int frequency, char* protocol)
{
	WSADATA wsaData;
	SOCKET ListenSocket, AcceptSocket;
	SOCKADDR_IN InternetAddr;
	DWORD ThreadId;
	DWORD RecvBytes;
	INT Ret;
	Container c;

	c.file = file;
	c.packet_size = packet_size;
	c.times = frequency;

	fin = FALSE;

	InitializeCriticalSection(&CriticalSection);

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		WSACleanup();
		return FALSE;
	}

	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return FALSE;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return FALSE;
	}

	if (listen(ListenSocket, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		return FALSE;
	}

	// Setup the listening socket for connections.

	if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return FALSE;
	}

	if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent failed with error %d\n", WSAGetLastError());
		return FALSE;
	}

	if (CreateThread(NULL, 0, ProcessIO, &c, 0, &ThreadId) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return FALSE;
	}

	EventTotal = 1;
	IncomingConnections(ListenSocket);
	return TRUE;
}

BOOL IncomingConnections(SOCKET ListenSocket)
{
	SOCKET AcceptSocket;
	DWORD Flags;
	DWORD RecvBytes;
	
	while (1)
	{
		// Accept inbound connections

		if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) == INVALID_SOCKET)
		{
			printf("accept failed with error %d\n", WSAGetLastError());
			return FALSE;
		}

		EnterCriticalSection(&CriticalSection);

		// Create a socket information structure to associate with the accepted socket.

		if ((SocketArray[EventTotal] = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return FALSE;
		}

		// Fill in the details of our accepted socket.

		SocketArray[EventTotal]->Socket = AcceptSocket;
		ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
		SocketArray[EventTotal]->BytesSEND = 0;
		SocketArray[EventTotal]->BytesRECV = 0;
		SocketArray[EventTotal]->DataBuf.len = DATA_BUFSIZE;
		SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;

		if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] =
			WSACreateEvent()) == WSA_INVALID_EVENT)
		{
			printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
			return FALSE;
		}

		// Post a WSARecv request to to begin receiving data on the socket

		Flags = 0;
		if (WSARecv(SocketArray[EventTotal]->Socket,
			&(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags,
			&(SocketArray[EventTotal]->Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return FALSE;
			}
		}

		EventTotal++;

		LeaveCriticalSection(&CriticalSection);

		//
		// Signal the first event in the event array to tell the worker thread to
		// service an additional event in the event array
		//
		if (WSASetEvent(EventArray[0]) == FALSE)
		{
			printf("WSASetEvent failed with error %d\n", WSAGetLastError());
			return FALSE;
		}
	}

	return TRUE;
}

DWORD WINAPI ProcessIO(LPVOID container)
{
	DWORD Index;
	DWORD Flags;
	LPSOCKET_INFORMATION SI;
	DWORD BytesTransferred;
	DWORD i;
	DWORD RecvBytes, SendBytes;
	DWORD read;
	BOOL last = FALSE;
	Container* c = (Container*)container;

	int size = c->packet_size;

	char buffer[4096] = { '\0' };
	int hits = 0;
	int sent = 0;
	int count = 0;

	// Process asynchronous WSASend, WSARecv requests.
	//ReadFile(fileSave, lol, haha, &read, NULL);
	
	while (1)
	{

		if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			printf("WSAWaitForMultipleEvents failed %d\n", WSAGetLastError());
			return 0;
		}

		// If the event triggered was zero then a connection attempt was made
		// on our listening socket.

		if ((Index - WSA_WAIT_EVENT_0) == 0)
		{
			WSAResetEvent(EventArray[0]);
			continue;
		}

		SI = SocketArray[Index - WSA_WAIT_EVENT_0];
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred,
			FALSE, &Flags) == FALSE || BytesTransferred == 0 || fin)
		{
			printf("Closing socket %d\n", SI->Socket);
			hits = 0;
			if (closesocket(SI->Socket) == SOCKET_ERROR)
			{
				printf("closesocket() failed with error %d\n", WSAGetLastError());
			}

			GlobalFree(SI);
			WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

			// Cleanup SocketArray and EventArray by removing the socket event handle
			// and socket information structure if they are not at the end of the
			// arrays.

			EnterCriticalSection(&CriticalSection);

			if ((Index - WSA_WAIT_EVENT_0) + 1 != EventTotal)
				for (i = Index - WSA_WAIT_EVENT_0; i < EventTotal; i++)
				{
					EventArray[i] = EventArray[i + 1];
					SocketArray[i] = SocketArray[i + 1];
				}

			EventTotal--;
			LeaveCriticalSection(&CriticalSection);
			return TRUE;
		}

		// Check to see if the BytesRECV field equals zero. If this is so, then
		// this means a WSARecv call just completed so update the BytesRECV field
		// with the BytesTransferred value from the completed WSARecv() call.

		if (SI->BytesRECV == 0)
		{
			SI->BytesRECV = BytesTransferred;
			SI->BytesSEND = 0;
		}
		else
		{
			SI->BytesSEND += BytesTransferred;
		}

		if (SI->BytesRECV > SI->BytesSEND)
		{
			
			if (ReadFile(c->file, buffer, c->packet_size, &read, NULL) && read < c->packet_size)
			{
				last = TRUE;
				SetFilePointer(c->file, NULL, NULL, FILE_BEGIN);
				count++;
			}
			
			// Post another WSASend() request.
			// Since WSASend() is not gauranteed to send all of the bytes requested,
			// continue posting WSASend() calls until all received bytes are sent.

			ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
			SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

			SI->DataBuf.buf = buffer;
			SI->DataBuf.len = read;
			if (SI->DataBuf.len == 0)
			{
				SI->DataBuf.buf[0] = '\0';
				SI->DataBuf.len = 1;
			}

			if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
				&(SI->Overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					printf("WSASend() failed with error %d\n", WSAGetLastError());
					return 0;
				}
			}
			else {
				hits++;
				
			}

			if (count >= c->times)
			{
				SI->DataBuf.buf[0] = '\0';
				SI->DataBuf.len = 1;
				WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
					&(SI->Overlapped), NULL);
				fin = TRUE;
				return TRUE;
				
			}
		}
		else
		{
			SI->BytesRECV = 0;

			// Now that there are no more bytes to send post another WSARecv() request.

			Flags = 0;
			ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
			SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];

			SI->DataBuf.len = DATA_BUFSIZE;
			SI->DataBuf.buf = SI->Buffer;

			if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
				&(SI->Overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					printf("WSARecv() failed with error %d\n", WSAGetLastError());
					return 0;
				}
			}
		}
	}
}*/
