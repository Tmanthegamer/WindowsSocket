/*=====================================================================================
SOURCE FILE:	Source.cpp

PROGRAM:		Networking with Windows

FUNCTIONS		TCHAR* GetTextFromInput();
				void SetOutputText(TCHAR* text, int len);
				void AppendOutputText(TCHAR* text);
				int WINAPI WinMain(	HINSTANCE hInst,
									HINSTANCE hprevInstance,
									LPSTR lspszCmdParam,
									int nCmdShow);
				LRESULT CALLBACK WndProc(
									HWND hwnd,
									UINT Message,
									WPARAM wParam,
									LPARAM lParam);

DATE:			January 16th, 2016

REVISIONS:		January 17th, 2016 (Tyler Trepanier-Bracken)
					Integrating Internet utitlies with this file.
				January 20th, 2016 (Tyler Trepanier-Bracken)
					Added menu items.

DESIGNER:		Tyler Trepanier-Bracken

PROGRAMMER		Tyler Trepanier-Bracken

NOTES:
The main entry point into the Networking with Windows program. This program contains
a menu having the options of changing the conversion method as well as clearing the
output text. 

There are short wrapper messages such as SetOutputText which sets the output text
and enhances readability of the program.

The program starts in WinMain which creates the main parent window where the
application will take place. Afterwards it will go into the forever function called
the Windows Procedure which will handle all windows events.
=====================================================================================*/

#include "Async.h"
#include "Source.h"
#include "resource.h"
#include <Mswsock.h>
#include <CommCtrl.h>

#pragma comment(lib, "Mswsock.lib")

HWND hwnd, inputHost, inputPort, combobox, convertBtn, clearBtn, inputGroupBox, listGroupBox, listview;
HANDLE fileRead = INVALID_HANDLE_VALUE, fileSave = INVALID_HANDLE_VALUE;
HMENU input_id = (HMENU)50;
HMENU ID = (HMENU)100;
HMENU combo_id = (HMENU)150;

HINSTANCE hInst;

static int avg_time		= 0;
static int packets_sent	= 0;
static int packets_lost	= 0;
static int total_data	= 0;
static int total_time	= 0;

char ip_text[20] = "";
short port = 0;
char protocol[4] = "TCP";
int packet_size = 1000;
int frequency = 1;

TCHAR Name[] = TEXT("The Amazing Convert-o-matic");

LPSOCKET_INFORMATION SocketInfoList;

#define WM_SOCKET (WM_USER + 1)

void CreateSocketInformation(SOCKET s);
LPSOCKET_INFORMATION GetSocketInformation(SOCKET s);
void FreeSocketInformation(SOCKET s);

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

HWND CreateListView(HWND parent)
{
	
	RECT rcClient;
	GetClientRect(parent, &rcClient);
	
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;

	int listviewWidth = width - 20;
	int listviewHeight = height - 150;

	HWND hListView = CreateWindowEx(
		0,
		WC_LISTVIEW,
		NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_SHOWSELALWAYS | LVS_REPORT,
		10, 100,
		listviewWidth,
		listviewHeight,
		parent, NULL, NULL, NULL);

	

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.pszText = "Total time";
	lvc.cx = listviewWidth/5;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(hListView, 0, &lvc);

	lvc.iSubItem = 0;
	lvc.pszText = "Total data";
	lvc.cx = listviewWidth / 5;
	ListView_InsertColumn(hListView, 0, &lvc);

	lvc.iSubItem = 0;
	lvc.pszText = "Packets Lost";
	lvc.cx = listviewWidth / 5;
	ListView_InsertColumn(hListView, 0, &lvc);

	lvc.iSubItem = 0;
	lvc.pszText = "Packets Sent";
	lvc.cx = listviewWidth / 5;
	ListView_InsertColumn(hListView, 0, &lvc);

	lvc.iSubItem = 0;
	lvc.pszText = "Avg Send Time";
	lvc.cx = (listviewWidth / 5) + 3;
	ListView_InsertColumn(hListView, 0, &lvc);

	/*LVITEM item;
	item.mask = LVIF_TEXT;
	item.cchTextMax = 6;

	item.iSubItem = 1;
	item.pszText = TEXT("12345");
	item.iItem = 0;
	ListView_InsertItem(hListView, &item);

	item.iSubItem = 2; // zero based index of column
	item.pszText = TEXT("23456");
	ListView_SetItem(hListView, &item);

	item.iSubItem = 3; // zero based index of column
	item.pszText = TEXT("34567");
	ListView_SetItem(hListView, &item);*/

	return hListView;
}

/* ==== Main program entry point. ==== */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;
	DWORD Ret;
	SOCKET Listen;
	SOCKADDR_IN InternetAddr;
	WSADATA wsaData;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, 
						Name, 
						WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
						CW_USEDEFAULT, 
						CW_USEDEFAULT,
						600, 
						500, 
						NULL, 
						NULL, 
						hInst,
						NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// Prepare echo server
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		return -1;
	}

	if ((Listen = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("socket() failed with error %d\n", WSAGetLastError());
		return -1;
	}

	WSAAsyncSelect(Listen, hwnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);

	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return -1;
	}

	if (listen(Listen, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		return -1;
	}

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return (int)Msg.wParam;
}

HANDLE saveFile() {
	TCHAR   szFile[MAX_PATH] = TEXT("\0");
	OPENFILENAME   ofn;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	memset(&(ofn), 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("Text (*.txt)\0*.txt\0");
	ofn.lpstrTitle = TEXT("Save File As");
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrDefExt = TEXT("txt");

	//get the filename the user wants to save to
	if (GetSaveFileName(&ofn))
	{
		DWORD dwTextLen = 0, bytesWritten = 0;

		//ofn.lpstrFile contains the full path of the file, get a handle to it
		hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;

		//save the contents into file
		/*if (WriteFile(hFile, buffer, strlen(buffer), &bytesWritten, NULL))
		{
		}*/

		//free resources
		//CloseHandle(hFile);
	}

	return hFile;
}

HANDLE selectFile() {
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	HANDLE hf;              // file handle

							// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn) == TRUE) {
		OutputDebugString(ofn.lpstrFile);
		hf = CreateFile(ofn.lpstrFile,
			GENERIC_READ,
			0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
		return hf;
	}
	return NULL;
}

HMENU CreateMenuOptions(void)
{
	
	HMENU hMenu = CreateMenu();

	HMENU hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_CONNECT, "&Connect");
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_SELECT, "&Select");
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_SAVE, "&Save");
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, "&Exit");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_PROTOCOL_TCP, "&TCP");
	AppendMenu(hSubMenu, MF_STRING, ID_PROTOCOL_UDP, "&UDP");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Protocol");

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_PACKETSIZE_1, "&1kB");
	AppendMenu(hSubMenu, MF_STRING, ID_PACKETSIZE_4, "&4kB");
	AppendMenu(hSubMenu, MF_STRING, ID_PACKETSIZE_20, "&20kB");
	AppendMenu(hSubMenu, MF_STRING, ID_PACKETSIZE_60, "&60kB");
	AppendMenu(hSubMenu, MF_STRING, ID_PACKETSIZE_CUSTOM, "&Custom");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Packet Size");

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_FREQUENCY_1, "&1 time");
	AppendMenu(hSubMenu, MF_STRING, ID_FREQUENCY_10, "&10 times");
	AppendMenu(hSubMenu, MF_STRING, ID_FREQUENCY_50, "&50 times");
	AppendMenu(hSubMenu, MF_STRING, ID_FREQUENCY_100, "&100 times");
	AppendMenu(hSubMenu, MF_STRING, ID_FREQUENCY_200, "&200 times");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Freqency");

	return hMenu;
}

BOOL sendfile(HANDLE file)
{
	return TRUE;
}

void CreateInputText(HWND hwnd)
{
	/* Container for the ComboBox drop down list. */
	listGroupBox = CreateWindow(TEXT("BUTTON"), TEXT("Enter Port Number"),
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		295, 10, 260, 50,
		hwnd,
		(HMENU)-1,
		hInst,
		NULL);

	/* Input Text field. */
	inputHost = CreateWindow(TEXT("EDIT"), TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 30, 250, 20,
		hwnd,
		(HMENU)IDD_EDIT_TEXT,
		hInst,
		NULL);

	/* Input Text field. */
	inputPort = CreateWindow(TEXT("EDIT"), TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		300, 30, 250, 20,
		hwnd,
		(HMENU)IDD_EDIT_TEXT,
		hInst,
		NULL);

	inputGroupBox = CreateWindow(TEXT("BUTTON"), TEXT("Enter IP Address"),
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_EX_TRANSPARENT,
		5, 10, 260, 50,
		hwnd,
		(HMENU)-1,
		hInst,
		NULL);

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HMENU hMenu = 0, hSubMenu;
	SOCKET Accept;
	static LPSOCKET_INFORMATION SocketInfo;
	DWORD RecvBytes, SendBytes;
	DWORD Flags;
	TCHAR* textInput;
	DWORD haha = 1000;
	DWORD read = 0;
	static int count = 0;
	TCHAR writeBuffer[MAXBUF] = { '\0' };
	static BOOL last = FALSE;
	static BOOL sending = FALSE;


	switch (Message)
	{
	case WM_CREATE:
		hMenu = CreateMenuOptions();
		listview = CreateListView(hwnd);
		CreateInputText(hwnd);

		updateStatistic(listview, 1, 2, 3, 4, 5);

		SetMenu(hwnd, hMenu);
		break;
	case WM_SOCKET:
		if (WSAGETSELECTERROR(lParam))
		{
			printf("Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
			FreeSocketInformation(wParam);
		}
		else
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_ACCEPT:
				char pants[100];
				if ((Accept = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
				{
					char pants[100];
					sprintf_s(pants, "%d", WSAGetLastError());
					OutputDebugString("accept() failed with error");
					OutputDebugString(pants);
					OutputDebugString("\n");
					break;
				}

				// Create a socket information structure to associate with the
				// socket for processing I/O.

				CreateSocketInformation(Accept);

				sprintf_s(pants, "%d", Accept);
				OutputDebugString("Socket number ");
				OutputDebugString(pants);
				OutputDebugString(" connected.\n");

				WSAAsyncSelect(Accept, hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);

				break;

			case FD_READ:

				SocketInfo = GetSocketInformation(wParam);

				// Read data only if the receive buffer is empty.

				if (SocketInfo->BytesRECV != 0)
				{
					SocketInfo->RecvPosted = TRUE;
					return 0;
				}
				else
				{
					SocketInfo->DataBuf.buf = '\0';
					SocketInfo->DataBuf.len = 1;

					Flags = 0;
					if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
						&Flags, NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSARecv() failed with error %d\n", WSAGetLastError());
							FreeSocketInformation(wParam);
							return 0;
						}
					}
					else // No error so update the byte count
					{
						SocketInfo->BytesRECV = RecvBytes;
						if (RecvBytes == 1 && SocketInfo->DataBuf.buf[0] == ACK)
						{
							PostMessage(hwnd, WM_SOCKET, wParam, FD_WRITE);
						}
					}
				}

				// DO NOT BREAK HERE SINCE WE GOT A SUCCESSFUL RECV. Go ahead
				// and begin writing data to the client.

			case FD_WRITE:

				if (fileRead == INVALID_HANDLE_VALUE)
				{
					OutputDebugString("Can't send file.\n");
					break;
				}


				if (count >= frequency) {
					break;
				}

				if (sending) {
					TransmitFile(SocketInfo->Socket, fileRead, packet_size, read, NULL, NULL, TF_REUSE_SOCKET);
				
					

					/*SocketInfo->DataBuf.buf = writeBuffer;
					SocketInfo->DataBuf.len = read;

					if (read == 0)
					{
						SocketInfo->DataBuf.buf[0] = EOT;
						SocketInfo->DataBuf.len = 1;
					}

					if (WSASend(SocketInfo->Socket,
						&(SocketInfo->DataBuf),
						1,
						&SendBytes,
						0,
						NULL, NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							OutputDebugString("WSASend() failed with error");
							sprintf_s(pants, "%d", WSAGetLastError());
							OutputDebugString(pants);
							OutputDebugString("\n");
							FreeSocketInformation(wParam);
							return 0;
						}
					}
					else // No error so update the byte count
					{
						SocketInfo->BytesSEND += SendBytes;
						packets_sent++;
					}


					if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
					{
						SocketInfo->BytesSEND = 0;
						SocketInfo->BytesRECV = 0;

						// If a RECV occurred during our SENDs then we need to post an FD_READ
						// notification on the socket.

						if (SocketInfo->RecvPosted == TRUE)
						{
							SocketInfo->RecvPosted = FALSE;
							PostMessage(hwnd, WM_SOCKET, wParam, FD_READ);
						}
					}*/
				}
				break;

			case FD_CLOSE:

				printf("Closing socket %d\n", wParam);
				FreeSocketInformation(wParam);
				WSACleanup();
				break;
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_PROTOCOL_TCP:
			sprintf_s(protocol, "TCP");
			break;
		case ID_PROTOCOL_UDP:
			sprintf_s(protocol, "UDP");
			break;
		case ID_PACKETSIZE_1:
			packet_size = 1000;
			break;
		case ID_PACKETSIZE_4:
			packet_size = 1000;
			break;
		case ID_PACKETSIZE_20:
			packet_size = 1000;
			break;
		case ID_PACKETSIZE_60:
			packet_size = 1000;
			break;
		case ID_PACKETSIZE_CUSTOM:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
			break;
		case ID_FREQUENCY_1:
			frequency = 1;
			break;
		case ID_FREQUENCY_10:
			frequency = 10;
			break;
		case ID_FREQUENCY_50:
			frequency = 50;
			break;
		case ID_FREQUENCY_100:
			frequency = 100;
			break;
		case ID_FREQUENCY_200:
			frequency = 200;
			break;
		case ID_FILE_CONNECT:
			GetTextFromHost();
			GetTextFromPort();
			sending = TRUE;
			PostMessage(hwnd, WM_SOCKET, wParam, FD_WRITE);
			//OpenPortForSending(fileRead, packet_size, frequency, protocol);
			break;
		case ID_FILE_SELECT:
			fileRead = selectFile();
			break;
		case ID_FILE_SAVE:
			fileSave = saveFile();
			break;
		case ID_FILE_EXIT:
			break;
		}
		break;

	case WM_DESTROY:	// Terminate program
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

void GetTextFromHost() {
	int len = SendMessage(inputHost, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* buffer = new char[len+1];
	SendMessage(inputHost, WM_GETTEXT, (WPARAM)len+1, (LPARAM)buffer);
	strncpy_s(ip_text, buffer, sizeof(ip_text));
}

void GetTextFromPort() {
	int len = SendMessage(inputPort, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* buffer = new char[len + 1];
	SendMessage(inputPort, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)buffer);
	sscanf_s(buffer, "%zu" , &port);
}

void SetOutputText(TCHAR* text, int len) {
	//SendMessage(output, WM_SETTEXT, (WPARAM)len, (LPARAM)text);
}

void AppendOutputText(TCHAR* text) {
	DWORD l, r;
	
	//SendMessage(output, EM_GETSEL, (WPARAM)&l, (LPARAM)&r);
	//SendMessage(output, EM_SETSEL, -1, -1);
	//SendMessage(output, EM_REPLACESEL, TRUE, (LPARAM)text);
}

void updateStatistic(HWND hList, int avg, int sent, int lost, int data, int time) {
	char a[10];
	char s[10];
	char l[10];
	char d[10];
	char t[10];
	sprintf_s(a, "%d", avg);
	sprintf_s(s, "%d", sent);
	sprintf_s(l, "%d", lost);
	sprintf_s(d, "%d", data);
	sprintf_s(t, "%d", time);
	ListView_SetItemText(hList, 1, 0, a);
	ListView_SetItemText(hList, 1, 1, s);
	ListView_SetItemText(hList, 1, 2, l);
	ListView_SetItemText(hList, 1, 3, d);
	ListView_SetItemText(hList, 1, 4, t);
}

void CreateSocketInformation(SOCKET s)
{
	LPSOCKET_INFORMATION SI;

	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return;
	}

	// Prepare SocketInfo structure for use.

	SI->Socket = s;
	SI->RecvPosted = FALSE;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;

	SI->Next = SocketInfoList;

	SocketInfoList = SI;
}

LPSOCKET_INFORMATION GetSocketInformation(SOCKET s)
{
	SOCKET_INFORMATION *SI = SocketInfoList;

	while (SI)
	{
		if (SI->Socket == s)
			return SI;

		SI = SI->Next;
	}

	return NULL;
}

void FreeSocketInformation(SOCKET s)
{
	SOCKET_INFORMATION *SI = SocketInfoList;
	SOCKET_INFORMATION *PrevSI = NULL;

	while (SI)
	{
		if (SI->Socket == s)
		{
			if (PrevSI)
				PrevSI->Next = SI->Next;
			else
				SocketInfoList = SI->Next;

			closesocket(SI->Socket);
			GlobalFree(SI);
			return;
		}

		PrevSI = SI;
		SI = SI->Next;
	}
}