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
HANDLE file_to_send = INVALID_HANDLE_VALUE, fileSave = INVALID_HANDLE_VALUE;
HMENU input_id = (HMENU)50;
HMENU ID = (HMENU)100;
HMENU combo_id = (HMENU)150;

HINSTANCE hInst;

static float avg_send_time = 0;
static float avg_recv_time = 0;
static int packets_sent = 0;
static int packets_recv = 0;
static double total_data_sent = 0;
static int total_data_recv = 0;
static long total_recv_time = 0;
static long total_send_time = 0;
static float avg_time = 0;
static int total_data = 0;
static int total_packets = 0;

char ip_text[20] = { '\0' };
short port = 5150;
int protocol = TCP;
int packet_size = 1000;
int frequency = 1;

BOOL Server = FALSE;
BOOL Client = FALSE;
BOOL sending_file = FALSE;
BOOL incoming_file = FALSE;
BOOL incoming_final_message = FALSE;
BOOL first_ack = FALSE;

TCHAR Name[] = TEXT("The Client");

LPSOCKET_INFORMATION SocketInfoList;
sockaddr sockAddrClient;
SOCKADDR_IN remote;
SOCKET test;

#define WM_SOCKET_TCP 225
#define WM_SOCKET_UDP 550
#define WM_CLIENT_TCP 220

long delay(SYSTEMTIME t1, SYSTEMTIME t2);
float avg_delay(long time, int num_packets);

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
	lvc.cx = listviewWidth / 6;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iOrder = 0;
	ListView_InsertColumn(hListView, 0, &lvc);

	lvc.iSubItem = 1;
	lvc.pszText = "Total data";
	lvc.cx = listviewWidth / 6;
	lvc.iOrder = 1;
	ListView_InsertColumn(hListView, 1, &lvc);

	lvc.iSubItem = 2;
	lvc.pszText = "Packets Recv";
	lvc.cx = listviewWidth / 6;
	lvc.iOrder = 2;
	ListView_InsertColumn(hListView, 2, &lvc);

	lvc.iSubItem = 3;
	lvc.pszText = "Packets Sent";
	lvc.cx = listviewWidth / 6;
	lvc.iOrder = 3;
	ListView_InsertColumn(hListView, 3, &lvc);

	lvc.iSubItem = 4;
	lvc.pszText = "Avg Send";
	lvc.cx = (listviewWidth / 6);
	lvc.iOrder = 4;
	ListView_InsertColumn(hListView, 4, &lvc);

	lvc.iSubItem = 5;
	lvc.pszText = "Avg Recv";
	lvc.cx = (listviewWidth / 6) + 3;
	lvc.iOrder = 5;
	ListView_InsertColumn(hListView, 5, &lvc);

	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.iGroup = 0;
	lvi.pszText = "bob";
	ListView_InsertItem(hListView, &lvi);

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

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return (int)Msg.wParam;
}

void SetServer(SOCKET* Accept) {
	MSG msg;
	DWORD Ret;
	SOCKET Listen;
	SOCKADDR_IN InternetAddr;
	HWND Window;
	WSADATA wsaData;
	int sock_type = 0;

	if (protocol == TCP)
	{
		sock_type = SOCK_STREAM;
	}
	else if (protocol == UDP)
	{
		sock_type = SOCK_DGRAM;
	}
	else
	{
		OutputDebugString("[[[ SET SERVER ERROR ]]]\n");
		OutputDebugString("protocol is not UDP or TCP, critical failure.\n");
		return;
	}

	// Prepare echo server
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		return;
	}

	if ((Listen = socket(PF_INET, sock_type, 0)) == INVALID_SOCKET)
	{
		printf("socket() failed with error %d\n", WSAGetLastError());
		return;
	}

	if (protocol == TCP)
	{
		WSAAsyncSelect(Listen, hwnd, WM_SOCKET_TCP, FD_ACCEPT | FD_CLOSE);
	}
	else if (protocol == UDP)
	{
		WSAAsyncSelect(Listen, hwnd, WM_SOCKET_UDP, FD_READ | FD_WRITE | FD_CLOSE);
	}


	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_port = htons(PORT);
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		OutputDebugString("[[[ SET SERVER ERROR ]]]\n");
		OutputDebugString("Unable to bind to socket.\n");
		return;
	}

	test = Listen;

	if (protocol == TCP)
	{
		if (listen(Listen, 5))
		{
			OutputDebugString("[[[ SET SERVER ERROR ]]]\n");
			OutputDebugString("Unable to TCP listen to socket.\n");
			return;
		}
	}
	else //Assuming the protocol is UDP
	{
		CreateSocketInformation(Listen);
	}

	OutputDebugString("Connected via Setserver\n");
}

void SetClientUDP()
{
	struct	hostent	*hp;
	struct	sockaddr_in server, client;
	WSADATA wsaData;
	SOCKET sock;
	int Ret;

	if (Server)
	{
		OutputDebugString("Cannot be a Server when it's already a Client.\n");
		return;
	}
	Client = TRUE;
	Server = FALSE;

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		return;
	}

	// Create a datagram socket
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Can't create a socket\n");
		return;
	}

	WSAAsyncSelect(sock, hwnd, WM_SOCKET_UDP, FD_READ | FD_WRITE | FD_CLOSE);

	// Store server's information
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if ((hp = gethostbyname(ip_text)) == NULL)
	{
		fprintf(stderr, "Can't get server's IP address\n");
		return;
	}
	//strcpy((char *)&server.sin_addr, hp->h_addr);
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Bind local address to the socket
	memset((char *)&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(0);  // bind to any available port
	client.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&client, sizeof(client)) == -1)
	{
		perror("Can't bind name to socket");
		return;
	}
	// Find out what port was assigned and print it
	int client_len = sizeof(client);
	if (getsockname(sock, (struct sockaddr *)&client, &client_len) < 0)
	{
		perror("getsockname: \n");
		return;
	}
	printf("Port assigned is %d\n", ntohs(client.sin_port));

	CreateSocketInformation(sock);
	test = sock;
	remote = server;
}

void SetClient(SOCKET* Accept)
{
	DWORD Ret;
	WSADATA wsaData;
	SOCKET sock;
	int protocol_type = 0;

	if (Server)
	{
		OutputDebugString("Cannot be a Server when it's already a Client.\n");
		return;
	}
	Client = TRUE;
	Server = FALSE;

	protocol_type = SOCK_STREAM;

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		return;
	}

	sock = socket(AF_INET, protocol_type, 0);
	if (sock == INVALID_SOCKET)
	{
		MessageBox(hwnd, "Socket creation failed", "Critical Error", MB_ICONERROR);
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return;
	}
	test = sock;
	WSAAsyncSelect(sock, hwnd, WM_CLIENT_TCP, FD_CLOSE | FD_READ);
}

void ClientConnect()
{
	SOCKADDR_IN SockAddr;
	SOCKADDR_IN ClientAddr;
	int client_len;

	struct hostent *host;
	if ((host = gethostbyname(ip_text)) == NULL)
	{
		MessageBox(hwnd,
			"Unable to resolve host name",
			"Critical Error",
			MB_ICONERROR);
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return;
	}

	// Set up our socket address structure

	SockAddr.sin_port = htons(port);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	connect(test, (LPSOCKADDR)(&SockAddr), sizeof(SockAddr));

	CreateSocketInformation(test);
}

HANDLE saveFile()
{
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
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_SEND, "&Send");
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

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_SERVER, "&Server");
	AppendMenu(hSubMenu, MF_STRING, ID_CLIENT, "&Client");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Server/Client");

	return hMenu;
}

BOOL appendtofile(HANDLE file, char* segment, int size)
{
	//char databuf[MAXBUF] = { '\0' };
	DWORD BytesWroteToFile = 0;

	if (WriteFile(file, segment, size, &BytesWroteToFile, NULL) == FALSE)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

std::string GetInitMessage(HANDLE file)
{
	long FileSize = 0;
	int totalPackets = 0;
	LARGE_INTEGER size;
	std::string temp;

	char initmsg[MAXBUF] = { '\0' };

	if (file == INVALID_HANDLE_VALUE || !GetFileSizeEx(file, &size))
	{
		//CloseHandle(file);
		return FALSE; // error condition, could call GetLastError to find out more
	}

	FileSize = size.QuadPart;
	totalPackets = FileSize / (packet_size - 1);
	if (FileSize % (packet_size - 1) > 0)
	{
		totalPackets += 1;
	}
	totalPackets *= frequency;

	sprintf_s(initmsg, "size: %d num: %d", packet_size, totalPackets);
	OutputDebugString(initmsg);
	temp = initmsg;
	return temp;
}

char** holder;
int real_total = 0;
std::vector<char*> GetPacketsFromFile(HANDLE file)
{
	char databuf[MAXBUF] = { '\0' };
	char c[10];

	std::vector<char*> packetarray;
	long FileSize = 0;
	int totalPackets = 0;
	LARGE_INTEGER size;
	std::string temp;
	DWORD BytesReadFromFile = 0;
	DWORD SendBytes = 0;
	DWORD ReadBytes = 0;
	int count = 0;

	if (file == INVALID_HANDLE_VALUE || !GetFileSizeEx(file, &size))
	{
		//CloseHandle(file);
		return packetarray; // error condition, could call GetLastError to find out more
	}

	FileSize = size.QuadPart;
	totalPackets = FileSize / (packet_size - 1);
	if (FileSize % (packet_size - 1) > 0)
	{
		totalPackets += 1;
	}
	
	if (real_total == 0)
		real_total = totalPackets * frequency;
	total_packets = totalPackets;
	
	holder = (char**)malloc(totalPackets * (packet_size + 1));
	
	while (count < totalPackets)
	{
		holder[count] = (char*)malloc(packet_size + 1);
		if (holder[count] == NULL) {
			OutputDebugString("{{{{shit}}}}");
		}

		memset(holder[count], 0, packet_size);
		count++;

	}
	count = 0;
	while (count < totalPackets)
	{
		if (ReadFile(file, holder[count], packet_size - 1, &BytesReadFromFile, NULL) == FALSE)
		{
			packetarray.empty();
			return packetarray;
		}
		else if (BytesReadFromFile < (packet_size - 1) || BytesReadFromFile == 0)
		{
			SetFilePointer(file, 0, NULL, FILE_BEGIN);

		}
		holder[count][packet_size - 1] = '\0';

		packetarray.push_back(holder[count]);
		count++;
	}
	SetFilePointer(file, 0, NULL, FILE_BEGIN);
	OutputDebugString("Sending finished.\n");
	return packetarray;
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
	inputHost = CreateWindow(TEXT("EDIT"), TEXT("192.168.0.2"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 30, 250, 20,
		hwnd,
		(HMENU)IDD_EDIT_TEXT,
		hInst,
		NULL);

	/* Input Text field. */
	inputPort = CreateWindow(TEXT("EDIT"), TEXT("5150"),
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
	static SOCKET Accept = 0;
	static LPSOCKET_INFORMATION SocketInfo;
	DWORD RecvBytes = 0, SendBytes = 0;
	DWORD Flags = 0;
	TCHAR* textInput;
	DWORD haha = 1000;
	DWORD read = 0;
	static std::vector<std::string> packets;
	static int count = 0;
	TCHAR writeBuffer[MAXBUF] = { '\0' };
	static BOOL finished = FALSE;
	static BOOL connected = FALSE;
	static BOOL temp_size = TRUE;
	static BOOL UDP_Init = TRUE;
	static BOOL first_send = FALSE;
	static BOOL final_send = FALSE;
	char datagram[MAXBUF] = { '\0' };
	int size = 0;
	static std::vector<char*> packets_to_send;
	static char current_packet[MAXBUF] = { '\0' };
	static char SendBigBuffer[MAXBUF] = { '\0' };
	static char* RecvBigBuffer = { '\0' };
	const SOCKADDR* temp = (SOCKADDR*)&remote;

	static int inc_packet_num = 0;
	static int inc_packet_size = 0;
	SYSTEMTIME stStartTime, stEndTime;

	switch (Message)
	{
	case WM_CREATE:
		hMenu = CreateMenuOptions();
		listview = CreateListView(hwnd);
		CreateInputText(hwnd);

		updateStatistic(listview, avg_send_time, avg_recv_time, packets_sent,
			packets_recv, total_data, total_recv_time + total_send_time);

		SetMenu(hwnd, hMenu);
		break;

	case WM_SOCKET_UDP:
		if (WSAGETSELECTERROR(lParam))
		{
			sprintf_s(datagram, "Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
			OutputDebugString(datagram);
			FreeSocketInformation(wParam);
			break;
		}

		if (UDP_Init)
		{
			RecvBigBuffer = new char[1000000];
			memset(RecvBigBuffer, 0, sizeof(RecvBigBuffer));
			UDP_Init = FALSE;

		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			OutputDebugString("[UDP]FD_READ:\n");
			SocketInfo = GetSocketInformation(wParam);

			if (SocketInfo == NULL)
			{
				OutputDebugString("Socket info is null in UDP read.\n");
				break;
			}

			if (temp_size)
			{
				inc_packet_size = 1000000;
			}

			SocketInfo->DataBuf.buf = RecvBigBuffer;
			SocketInfo->DataBuf.len = 1000000;
			Flags = 0;

			GetSystemTime(&stStartTime);
			if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
				&Flags, NULL, NULL, NULL, NULL) == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK)
				{
					OutputDebugString("WSARecvFrom: Error WSAEWOULDBLOCK.\n");
					/*printf("WSARecv() failed with error %d\n", WSAGetLastError());
					FreeSocketInformation(wParam);
					return 0;*/
				}
			}
			GetSystemTime(&stEndTime);

			if (RecvBytes > 0)
			{
				packets_recv++;
				total_data_recv += (RecvBytes / 1000);
				total_recv_time += delay(stStartTime, stEndTime);
				avg_recv_time = avg_delay(total_recv_time, packets_recv);
			}

			if (temp_size)
			{
				temp_size = FALSE;
				inc_packet_size = (int)RecvBytes;
			}
			else
			{
				inc_packet_size = 0;
			}

			break;

		case FD_WRITE:

			OutputDebugString("[UDP]FD_WRITE\n");

			SocketInfo = GetSocketInformation(wParam);

			if (SocketInfo == NULL)
			{
				OutputDebugString("Socket info is null in UDP read.\n");
				break;
			}

			if (sending_file && packets_to_send.size() > 0)
			{
				SocketInfo->DataBuf.len = 7;
				sprintf_s(current_packet, "%s", packets_to_send.front());
				packets_to_send.erase(packets_to_send.begin());
			}
			else
			{
				memset(current_packet, 0, sizeof(current_packet));
			}
			SocketInfo->DataBuf.buf = current_packet;
			SocketInfo->DataBuf.len = strlen(current_packet) + 1;

			Flags = 0;

			GetSystemTime(&stStartTime);

			if(WSASendTo(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, Flags, temp, sizeof(remote), NULL, NULL))
			{
				int err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK)
				{
					OutputDebugString("WSASendTo: WSAEWOULDBLOCK\n");
				}
			}
			GetSystemTime(&stEndTime);

			sprintf_s(current_packet, "%d ", SocketInfo->DataBuf.len);
			OutputDebugString(current_packet);

			packets_sent++;
			total_data_sent += (SocketInfo->DataBuf.len / 1000);
			total_send_time += delay(stStartTime, stEndTime);
			avg_send_time = avg_delay(total_send_time, packets_sent);

			if (sending_file)
			{
				if (packets_to_send.size() > 0)
				{
					PostMessage(hwnd, Message, wParam, lParam);
				}
				else {
					frequency--;
					if (frequency > 0)
					{
						
						for (int i = total_packets - 1; i > -1; i--)
						{
							free(holder[i]);
						}
						free(holder);
						packets_to_send = GetPacketsFromFile(file_to_send);
						PostMessage(hwnd, Message, wParam, lParam);
					}
				}
			}

			/*
			Serves no purpose other than allow the server some time
			to process the sent messages.

			THIS IS NECESSARY, removing this section will result in a
			buffer overflow for the reader.
			*/
			for (int i = 0; i < MAXBUF; i++)
			{
				SendBigBuffer[i] = '\0';
			}
			for (int i = 0; i < MAXBUF; i++)
			{
				SendBigBuffer[i] = '\0';
			}

			break;

		case FD_CLOSE:
			OutputDebugString("[UDP]FD_CLOSE\n");
			printf("Closing socket %d\n", wParam);
			FreeSocketInformation(wParam);

			if (holder != NULL)
			{
				free(holder);
			}
			break;
		}
		memset(SendBigBuffer, 0, sizeof(SendBigBuffer));

		total_data = total_data_recv + total_data_sent;
		updateStatistic(listview, avg_send_time, avg_recv_time, packets_sent,
			packets_recv, total_data, total_recv_time + total_send_time);

		break;

	case WM_SOCKET_TCP:
		if (WSAGETSELECTERROR(lParam))
		{
			sprintf_s(datagram, "Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
			OutputDebugString(datagram);
			FreeSocketInformation(wParam);
			break;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			size = sizeof(sockaddr);
			if ((Accept = accept(wParam, &sockAddrClient, &size)) == INVALID_SOCKET)
			{
				MessageBox(hwnd, "Can't accept client.", "Critical Error", MB_ICONERROR);
				break;
			}
			WSAAsyncSelect(Accept, hwnd, WM_SOCKET_TCP, FD_READ | FD_WRITE | FD_CLOSE);
			OutputDebugString("Accepted the client!\n");

			// Create a socket information structure to associate with the
			// socket for processing I/O.
			CreateSocketInformation(Accept);
			break;

		case FD_READ:
			OutputDebugString("[TCP]FD_READ\n");

			SocketInfo = GetSocketInformation(wParam);

			if (SocketInfo == NULL)
			{
				OutputDebugString("Socketinfo is null in SOCKET_TCP\n");
				break;
			}

			// Read data only if the receive buffer is empty.

			if (SocketInfo->BytesRECV != 0)
			{
				SocketInfo->RecvPosted = TRUE;
				OutputDebugString("Not finished, I guess\n");
			}
			else
			{
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				SocketInfo->DataBuf.len = DATA_BUFSIZE;

				Flags = 0;
				GetSystemTime(&stStartTime);
				if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
					&Flags, NULL, NULL) == SOCKET_ERROR)
				{
					int err = WSAGetLastError();
					if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						err,
						0,
						datagram,
						0,
						NULL) != 0)
					{
						OutputDebugString("{{WMSOCKETTCP:RECV:");
						OutputDebugString(datagram);
						OutputDebugString("}}\n");
					}

					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						FreeSocketInformation(wParam);
						OutputDebugString("WSASend failure.");
						return 0;
					}
				}
				else // No error so update the byte count
				{
					SocketInfo->BytesRECV = RecvBytes;
				}
				GetSystemTime(&stEndTime);

				total_data_recv += (RecvBytes / 1000);
				packets_recv++;
				count++;
				total_recv_time += delay(stStartTime, stEndTime);
				avg_recv_time = avg_delay(total_send_time, packets_sent);

				if (!incoming_file && sscanf_s(SocketInfo->DataBuf.buf, "size: %d num: %d", &inc_packet_size, &inc_packet_num) == 2)
				{
					incoming_file = TRUE;
					first_ack = TRUE;
					count = 0;
					sprintf_s(datagram, "[[[Init Message:%s][Size:%d]]\n", SocketInfo->DataBuf.buf, RecvBytes);
					OutputDebugString(datagram);
					//PostMessage(hwnd, WM_SOCKET_TCP, wParam, FD_READ);
				}
				else if (incoming_file && SocketInfo->DataBuf.buf[0] == EOT)
				{
					incoming_file = FALSE;
					incoming_final_message = TRUE;
					sprintf_s(datagram, "[[[EOT][Size:%d]]\n", RecvBytes);
				}
				else if (incoming_file)
				{
					sprintf_s(datagram, "[[[size:%d][remaining:%d]\n[", RecvBytes, (inc_packet_num - count));
					OutputDebugString(datagram);
					OutputDebugString(SocketInfo->DataBuf.buf);
					OutputDebugString("]]]\n");

					appendtofile(fileSave, SocketInfo->DataBuf.buf, strlen(SocketInfo->DataBuf.buf));

				}
			}
			total_data = total_data_recv + total_data_sent;
			avg_time = avg_delay(total_recv_time + total_send_time, packets_sent + packets_recv);

			updateStatistic(listview, avg_send_time, avg_recv_time, packets_sent,
				packets_recv, total_data, total_recv_time + total_send_time);

		case FD_WRITE:
			OutputDebugString("[TCP]FD_WRITE\n");

			SocketInfo = GetSocketInformation(wParam);
			if (SocketInfo == NULL || (sending_file && incoming_file))
			{
				OutputDebugString("Socket Info is null\n");
				break;
			}

			if (incoming_file && SocketInfo->BytesRECV > SocketInfo->BytesSEND)
			{
				if (first_ack)
				{
					memset(SocketInfo->Buffer, SOT, sizeof(SocketInfo->Buffer));
				}
				else if (incoming_final_message)
				{
					memset(SocketInfo->Buffer, EOT, sizeof(SocketInfo->Buffer));
				}
				else 
				{
					memset(SocketInfo->Buffer, ACK, sizeof(SocketInfo->Buffer));
				}
				SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
				SocketInfo->DataBuf.len = 5;

				GetSystemTime(&stStartTime);
				if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
					NULL, NULL) == SOCKET_ERROR)
				{
					int err = WSAGetLastError();
					if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						err,
						0,
						datagram,
						0,
						NULL) != 0)
					{
						OutputDebugString("{{WMSOCKETTCP:SEND:");
						OutputDebugString(datagram);
						OutputDebugString("}}\n");
					}

					if (err != WSAEWOULDBLOCK)
					{
						OutputDebugString("WSASend() failed\n");
						FreeSocketInformation(wParam);
						return 0;
					}
				}
				else // No error so update the byte count
				{
					SocketInfo->BytesSEND += SendBytes;
				}
				GetSystemTime(&stEndTime);

				total_data_sent += (SendBytes / 1000);
				packets_sent++;
				total_send_time += delay(stStartTime, stEndTime);
				avg_send_time = avg_delay(total_send_time, packets_sent);

			}

			if (SocketInfo->BytesSEND >= 5)
			{
				SocketInfo->BytesSEND = 0;
				SocketInfo->BytesRECV = 0;

				if (first_ack)
				{
					first_ack = FALSE;
				}

				// If a RECV occurred during our SENDs then we need to post an FD_READ
				// notification on the socket.

				if (SocketInfo->RecvPosted == TRUE)
				{
					SocketInfo->RecvPosted = FALSE;
					PostMessage(hwnd, WM_SOCKET_TCP, wParam, FD_READ);
				}
			}

			updateStatistic(listview, avg_send_time, avg_recv_time, packets_sent,
				packets_recv, total_data, total_recv_time + total_send_time);

			break;

		case FD_CLOSE:
			OutputDebugString("[TCP]FD_CLOSE\n");
			first_send = FALSE;
			printf("Closing socket %d\n", wParam);
			FreeSocketInformation(wParam);

			break;
		}
		break;

	case WM_CLIENT_TCP:
		if (WSAGETSELECTERROR(lParam))
		{
			printf("Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
			FreeSocketInformation(wParam);
			break;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			OutputDebugString("[TCP]FD_READ\n");

			SocketInfo = GetSocketInformation(wParam);

			if (SocketInfo == NULL)
			{
				OutputDebugString("Socket info is null in CLIENT_TCP\n");
				break;
			}

			// Read data only if the receive buffer is empty.

			if (SocketInfo->BytesRECV != 0)
			{
				SocketInfo->RecvPosted = TRUE;
				OutputDebugString("Shit happened");
			}
			else
			{
				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				SocketInfo->DataBuf.len = DATA_BUFSIZE;

				Flags = 0;
				GetSystemTime(&stStartTime);
				if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes,
					&Flags, NULL, NULL) == SOCKET_ERROR)
				{
					int err = WSAGetLastError();
					if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						err,
						0,
						datagram,
						0,
						NULL) != 0)
					{
						OutputDebugString("{{WM_CLIENT_TCP:recv:");
						OutputDebugString(datagram);
						OutputDebugString("}}\n");
					}

					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						FreeSocketInformation(wParam);
						OutputDebugString("WSASend failure.");
						return 0;
					}
				}
				else // No error so update the byte count
				{
					SocketInfo->BytesRECV = RecvBytes;
				}
				GetSystemTime(&stEndTime);

				total_data_recv += (RecvBytes / 1000);
				packets_recv++;
				count++;
				total_recv_time += delay(stStartTime, stEndTime);
				avg_recv_time = avg_delay(total_send_time, packets_sent);

				if (first_send && SocketInfo->BytesRECV > 0 && SocketInfo->DataBuf.buf[0] == SOT)
				{
					sending_file = TRUE;
					first_send = FALSE;
					packets_to_send = GetPacketsFromFile(file_to_send);
					sprintf_s(current_packet, "%s", packets_to_send.front());
					packets_to_send.erase(packets_to_send.begin());
					OutputDebugString("[[[Begin send]]]");
				}
				else if (sending_file && SocketInfo->BytesRECV > 0)
				{
					if (SocketInfo->DataBuf.buf[0] == ACK)
					{
						memset(SocketInfo->Buffer, 0, sizeof(SocketInfo->Buffer));
						if (packets_to_send.size() > 0)
						{
							sprintf_s(current_packet, "%s", packets_to_send.front());
							packets_to_send.erase(packets_to_send.begin());
							
							OutputDebugString(datagram);
						}
						else
						{
							frequency--;
							memset(current_packet, 0, sizeof(current_packet));
							if (frequency > 0)
							{
								for (int i = total_packets - 1; i > -1; i--)
								{
									if(holder[i] != NULL)
										free(holder[i]);
								}
								free(holder);
								packets_to_send = GetPacketsFromFile(file_to_send);
								sprintf_s(current_packet, "%s", packets_to_send.front());
								packets_to_send.erase(packets_to_send.begin());
								OutputDebugString("[[[Restart file send.]]]\n");
							}
							else
							{
								sending_file = FALSE;
								OutputDebugString("Sending EOT\n");
								memset(current_packet, EOT, packet_size);
								final_send = TRUE;
								PostMessage(hwnd, WM_CLIENT_TCP, wParam, FD_CLOSE);
							}

						}
						sprintf_s(SocketInfo->Buffer, "%s", current_packet);
					}
					else if (SocketInfo->DataBuf.buf[0] == EOT)
					{
						OutputDebugString("[[Sending EOT]]\n");
						sending_file = FALSE;
						memset(SocketInfo->DataBuf.buf, 0, sizeof(SocketInfo->DataBuf.buf));
						memset(current_packet, 0, sizeof(current_packet));
						memset(SocketInfo->Buffer, 0, sizeof(SocketInfo->Buffer));
						PostMessage(hwnd, WM_CLIENT_TCP, wParam, FD_CLOSE);
					}
				}
			}
			total_data = total_data_recv + total_data_sent;
			avg_time = avg_delay(total_recv_time + total_send_time, packets_sent + packets_recv);

			updateStatistic(listview, avg_send_time, avg_recv_time, packets_sent,
				packets_recv, total_data, total_recv_time + total_send_time);

			/*
			DO NOT BREAK HERE SINCE WE GOT A SUCCESSFUL RECV. Go ahead
			and begin writing data to the client.
			*/
		case FD_WRITE:
			OutputDebugString("[TCP]FD_WRITE\n");

			SocketInfo = GetSocketInformation(wParam);
			if (SocketInfo == NULL || (sending_file && incoming_file))
			{
				OutputDebugString("Socket info is null\n");
				break;
			}

			if(sending_file)
			{
				memset(SocketInfo->Buffer, 0, sizeof(SocketInfo->Buffer));
				sprintf_s(SocketInfo->Buffer, "%s", current_packet);
			}

			if (!sending_file && final_send) {
				memset(SocketInfo->Buffer, 0, sizeof(SocketInfo->Buffer));
				sprintf_s(SocketInfo->Buffer, "%s", current_packet);
				final_send = FALSE;
			}

			if (SocketInfo->BytesRECV > SocketInfo->BytesSEND)
			{
				
				SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
				SocketInfo->DataBuf.len = packet_size - SocketInfo->BytesSEND;

				GetSystemTime(&stStartTime);
				if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,
					NULL, NULL) == SOCKET_ERROR)
				{
					int err = WSAGetLastError();
					switch (err)
					{
					case WSAECONNABORTED:
						OutputDebugString("WSAECONNABORTED");
						break;
					case WSAECONNRESET:
						OutputDebugString("WSAECONNRESET");
						break;
					case WSAEFAULT:
						OutputDebugString("WSAEFAULT");
						break;
					case WSA_OPERATION_ABORTED:
						OutputDebugString("WSA_OPERATION_ABORTED");
						break;
					case WSAEINTR:
						OutputDebugString("WSAEINTR");
						break;
					case WSAEINPROGRESS:
						OutputDebugString("WSAEINPROGRESS");
						break;
					case WSAEINVAL:
						OutputDebugString("WSAEINVAL");
						break;
					case WSAEMSGSIZE:
						OutputDebugString("WSAEMSGSIZE");
						break;
					case WSAENETDOWN:
						OutputDebugString("WSAENETDOWN");
						break;
					case WSAENETRESET:
						OutputDebugString("WSAENETRESET");
						break;
					case WSAENOBUFS:
						OutputDebugString("WSAENOBUFS");
						break;
					case WSAENOTCONN:
						OutputDebugString("WSAENOTCONN");
						break;
					case WSAENOTSOCK:
						OutputDebugString("WSAENOTSOCK");
						break;
					case WSAEOPNOTSUPP:
						OutputDebugString("WSAEOPNOTSUPP");
						break;
					case WSAEALREADY:
						OutputDebugString("jk");
						break;
					case WSAESHUTDOWN:
						OutputDebugString("WSAESHUTDOWN");
						break;
					case WSAEWOULDBLOCK:
						OutputDebugString("WSAEWOULDBLOCK");
						PostMessage(hwnd, Message, wParam, FD_WRITE);
						break;
					case WSANOTINITIALISED:
						OutputDebugString("WSANOTINITIALISED");
						break;
					case WSAEPROTOTYPE:
						OutputDebugString("WSAEPROTOTYPE");
						break;
					case WSAENOPROTOOPT:
						OutputDebugString("WSAENOPROTOOPT");
						break;
					case WSAEPROTONOSUPPORT:
						OutputDebugString("WSAEPROTONOSUPPORT");
						break;
					case WSA_IO_PENDING:
						OutputDebugString("WSA_IO_PENDING");
						break;

					}
					
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						FreeSocketInformation(wParam);
						OutputDebugString("WSASend failure.");
						return 0;
					}
				}
				else // No error so update the byte count
				{
					SocketInfo->BytesSEND += SendBytes;
				}
				GetSystemTime(&stEndTime);

				sprintf_s(datagram, "Message:[[[SendBytes:%d][PacketSize:%d\n[", SendBytes, packet_size);
				OutputDebugString(datagram);
				

				
				total_send_time += delay(stStartTime, stEndTime);

			}

			if (SocketInfo->BytesSEND == packet_size)
			{
				total_data_sent += (SocketInfo->BytesSEND / 1000);
				SocketInfo->BytesSEND = 0;
				SocketInfo->BytesRECV = 0;

				packets_sent++;
				sprintf_s(datagram, "[total packets:%d][remaining:%d]\n", real_total, (real_total - packets_sent));
				OutputDebugString(datagram);
				avg_send_time = avg_delay(total_send_time, packets_sent);
				// If a RECV occurred during our SENDs then we need to post an FD_READ
				// notification on the socket.

				if (SocketInfo->RecvPosted == TRUE)
				{
					SocketInfo->RecvPosted = FALSE;
					PostMessage(hwnd, WM_CLIENT_TCP, wParam, FD_READ);
				}
			}
			else
			{
				sprintf_s(datagram, "<<%d>>", SocketInfo->BytesSEND);
				PostMessage(hwnd, WM_CLIENT_TCP, wParam, FD_WRITE);
				OutputDebugString(datagram);
			}

			updateStatistic(listview, avg_send_time, avg_recv_time, packets_sent,
				packets_recv, total_data, total_recv_time + total_send_time);

			break;

		case FD_CLOSE:
			OutputDebugString("[TCP]FD_CLOSE\n");
			first_send = FALSE;
			printf("Closing socket %d\n", wParam);
			FreeSocketInformation(wParam);

			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_PROTOCOL_TCP:
			protocol = TCP;
			break;
		case ID_PROTOCOL_UDP:
			protocol = UDP;
			OutputDebugString("[[UDP selected]]\n");
			break;
		case ID_PACKETSIZE_1:
			packet_size = 1000;
			break;
		case ID_PACKETSIZE_4:
			packet_size = 4000;
			break;
		case ID_PACKETSIZE_20:
			packet_size = 20000;
			break;
		case ID_PACKETSIZE_60:
			packet_size = 60000;
			break;
		case ID_PACKETSIZE_CUSTOM:
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
		case ID_CLIENT:
			GetTextFromHost();
			GetTextFromPort();
			OutputDebugString(ip_text);
			if (protocol == TCP)
			{
				SetClient(&Accept);
				ClientConnect();
			}
			else if (protocol == UDP)
			{
				SetClientUDP();

			}
			OutputDebugString("Client setting done.\n");
			break;
		case ID_SERVER:
			GetTextFromHost();
			GetTextFromPort();
			char haha[100];
			sprintf_s(haha, "port: %zu ip:%s", port, ip_text);

			OutputDebugString(haha);
			SetServer(&Accept);
			break;
		case ID_FILE_SEND:
			if (!Client)
				break;
			if (protocol == TCP)
			{
				memset(datagram, 0, sizeof(datagram));
				sprintf_s(datagram, "%s", GetInitMessage(file_to_send).c_str());
				SocketInfo = GetSocketInformation(test);
				first_send = TRUE;
				send(test, datagram, strlen(datagram) + 1, 0);
			}
			else if (protocol == UDP)
			{
				packets_to_send = GetPacketsFromFile(file_to_send);
				inc_packet_num = packets_to_send.size() * frequency;
				PostMessage(hwnd, WM_SOCKET_UDP, test, FD_WRITE);
				sending_file = TRUE;
			}

			break;

		case ID_FILE_SELECT:
			file_to_send = selectFile();
			break;
		case ID_FILE_SAVE:
			fileSave = saveFile();
			break;
		case ID_FILE_EXIT:
			SendMessage(hwnd, WM_DESTROY, NULL, NULL);
			break;
		}
		break;

	case WM_DESTROY:	// Terminate program
		CloseHandle(file_to_send);
		CloseHandle(fileSave);

		if (holder != NULL)
			free(holder);

		WSACleanup();
		PostQuitMessage(0);
		break;

	}
	return DefWindowProc(hwnd, Message, wParam, lParam);;
}

void GetTextFromHost() {
	int len = SendMessage(inputHost, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* buffer = new char[len + 1];
	SendMessage(inputHost, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)buffer);
	strncpy_s(ip_text, buffer, sizeof(ip_text));
}

void GetTextFromPort() {
	int temp = protocol;
	
	int len = SendMessage(inputPort, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* buffer = new char[len + 1];
	SendMessage(inputPort, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)buffer);
	sscanf_s(buffer, "%zu", &port);

	protocol = temp;
}

void updateStatistic(HWND hList, float savg, float ravg, int sent, int recv, double data, int time) {
	char sa[100];
	char ra[100];
	char s[100];
	char r[100];
	char d[100] = "0";
	char t[100];

	
	sprintf_s(sa, "%.6f ms", savg);
	sprintf_s(ra, "%.6f ms", ravg);
	sprintf_s(s, "%d", sent);
	sprintf_s(r, "%d", recv);
	double temp = data / 1000;
	if (temp > 0)
	{
		sprintf_s(d, "%lf MB", temp);
	}
	
	sprintf_s(t, "%d ms", time);
	ListView_SetItemText(hList, 0, 0, t);
	ListView_SetItemText(hList, 0, 1, d);
	ListView_SetItemText(hList, 0, 2, r);
	ListView_SetItemText(hList, 0, 3, s);
	ListView_SetItemText(hList, 0, 4, sa);
	ListView_SetItemText(hList, 0, 5, ra);
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

// Compute the delay between tl and t2 in milliseconds
long delay(SYSTEMTIME t1, SYSTEMTIME t2)
{
	long d;

	d = (t2.wSecond - t1.wSecond) * 1000;
	d += (t2.wMilliseconds - t1.wMilliseconds);
	return(d);
}

// Computer the average time for packet transmission
float avg_delay(long time, int num_packets)
{
	float avg = ((float)time) / ((float)num_packets);

	return avg;
}