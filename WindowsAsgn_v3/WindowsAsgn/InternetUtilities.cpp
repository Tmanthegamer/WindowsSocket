/*=====================================================================================
SOURCE FILE:	InternetUtilities.h

PROGRAM:		Networking with Windows

FUNCTIONS		void AnalyzeInput(char* text, int option);
				BOOL HostToIpAddress(char* host);
				void AppendOutputText(TCHAR* text);
				BOOL IpAddressToHost(char* ip);
				BOOL ServiceToPort(std::vector<char*> v);
				BOOL PortToService(std::vector<char*> v);

DATE:			January 16th, 2016

REVISIONS:		January 17th, 2016 (Tyler Trepanier-Bracken)
Organized header file into a cleaner format.

DESIGNER:		Tyler Trepanier-Bracken

PROGRAMMER		Tyler Trepanier-Bracken

NOTES:
NOTES:
Thee purpose of this header file is to contain all the utilities necessary to parse 
the text input received by the User and assign them to their assigned function based 
on the user's preference.

All errors will be displayed on the output text field. In addition, it will display
the usage instructions on ill-formed input.
=====================================================================================*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>

#include "InternetUtilities.h"
#include "Source.h"

#pragma comment(lib, "Ws2_32.lib")

/* ==== Separate the text by delimiters and guide those arguements	==== 
   ==== to the designated function.									====*/
void AnalyzeInput(char* text, int option)
{
	
	std::vector<char*> args;
	char* split;
	char* delim = " ";
	int num = 0;

	char *next_token1 = NULL;
	split = strtok_s(text, delim, &next_token1);
	while ((split != NULL))
	{
		if (args.size() > 3) {
			AppendOutputText("Maximum of two arguments only.\r\n");
			break;
		}
		args.push_back(split);
		split = strtok_s(NULL, delim, &next_token1);
	}

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		AppendOutputText("Error opening WinSock. Check your internet connection...");
		return;
	}

	switch(option)
	{
	case IP_TO_HOST:
		if (args.size() != 1) {
			AppendOutputText("Error, improper format. \r\n\tUsage: \"#.#.#.#\"\r\n");
			return;
		}
		// IP Address and Hosting
		if (!IpAddressToHost(args[0]))	// Dotted IP?
		{
			AppendOutputText("IpAddressToHost failure.");
			return;
		}
		break;
	case HOST_TO_IP:
		if (args.size() != 1) {
			AppendOutputText("Error, improper format. Use the format below.\r\n\te.g. \"bcit.ca\"\r\n");
			return;
		}
		if (!HostToIpAddress(args[0]))
		{
			AppendOutputText("HostToIpAddress failure.");
			return;
		}
		break;
	case SERVICE_TO_PORT:
		if (args.size() != 2)
		{
			AppendOutputText("Error, improper format. Use the format below.\r\n\tUsage: \"<Service> <Protocol>\"\r\n");
			return;
		}

		if (!ServiceToPort(args)) {
			AppendOutputText("ServiceToPort failure.");
			return;
		}

		break;
	case PORT_TO_SERVICE:
		if (args.size() != 2)
		{
			AppendOutputText("Error, improper format. Use the format below.\r\n\tUsage: \"<Port #> <Protocol>\"\r\n");
			return;
		}
		
		if (!PortToService(args)) {
			AppendOutputText("PortToService failure.");
			return;
		}
		break;
	}
	
}

/* ==== Convert a string representing a IP Address to its domain ==== */
BOOL IpAddressToHost(char* ip)
{
	int		a;					//Used to verify proper string format.
	hostent *hp;				//Container for addressing information.
	in_addr my_addr;			//Container used to hold host information.
	in_addr *addr_p;			//Pointer to a containuer that holds 
								//  host information.
	char	**p;				//Dynamic pointer that helps with displaying
								//	host information.
	char	ip_address[256];    // String for IP address
	
	
	addr_p = (struct in_addr*)malloc(sizeof(struct in_addr));
	if (addr_p == NULL) {
		AppendOutputText("Memory allocation failure. It's probably your fault.");
		return FALSE;
	}
	addr_p = &my_addr;

	
	if (!isdigit(ip[0]) || (a = inet_addr(ip)) == 0)
	{
		AppendOutputText("IP Address must be numbers in the form #.#.#.#\r\n");
		return FALSE;
	}

	// Copy IP address  into ip_address
	strcpy_s(ip_address, ip);
	addr_p->s_addr = inet_addr(ip_address);

	hp = gethostbyaddr((char *)addr_p, PF_INET, sizeof(my_addr));

	// Host failure.
	if (hp == NULL)
	{
		AppendOutputText("host information for");
		AppendOutputText(ip);
		AppendOutputText("not found\r\n");
		return FALSE;
	}
	
	// Print the contents of all available hosts.
	for (p = hp->h_addr_list; *p != 0; p++)
	{
		in_addr in;
		char **q;

		memcpy(&in.s_addr, *p, sizeof(in.s_addr));

		AppendOutputText("IP Address: ");
		AppendOutputText(inet_ntoa(in));
		AppendOutputText("\t Host Name: ");
		AppendOutputText(hp->h_name);
		AppendOutputText(" \n");
			
		for (q = hp->h_aliases; *q != 0; q++) {
			AppendOutputText(" \t\t\t\t   Aliases: ");
			AppendOutputText(*q);
			AppendOutputText("\n");
		}
				
	}

	return TRUE;
}

/* ==== Convert a string representing a host to its various IP addresses. ==== */
BOOL HostToIpAddress(char* host)
{
	hostent *hp;		//Container for addressing information.
	in_addr my_addr;	//Container used to hold host information.
	in_addr *addr_p;	//Pointer to a containuer that holds 
						//  host information.
	char	**p;		//Dynamic pointer that helps with displaying
						//	host information.

	addr_p = (struct in_addr*)malloc(sizeof(struct in_addr));
	addr_p = &my_addr;
	if (addr_p == NULL) {
		AppendOutputText("Memory allocation failure. It's probably your fault.");
		return FALSE;
	}
	
	char *errors[] = {	"No such host\r\n",
						"Bad host, try again later\r\n",
						"DNS Error with host, try again later\r\n",
						"Host has no IP address\r\n",
						"Unknown Error\r\n" };

	if ((hp = gethostbyname(host)) == NULL)
	{
		switch (h_errno)
		{
		case HOST_NOT_FOUND:
			AppendOutputText(errors[0]);
			break;
		case TRY_AGAIN:
			AppendOutputText(errors[1]);
			break;
		case NO_RECOVERY:
			AppendOutputText(errors[2]);
			break;
		case NO_ADDRESS:
			AppendOutputText(errors[3]);
			break;
		default:
			AppendOutputText(errors[4]);
			break;
		}
		return FALSE;
	}
		
	for (p = hp->h_addr_list; *p != 0; p++)
	{
		in_addr in;
		char **q;
			
		memcpy(&in.s_addr, *p, sizeof(in.s_addr));

		AppendOutputText("IP Address: ");
		AppendOutputText(inet_ntoa(in));
		AppendOutputText("\t Host Name: ");
		AppendOutputText(hp->h_name);
		AppendOutputText(" \r\n");

		for (q = hp->h_aliases; *q != 0; q++) {
			AppendOutputText(" \t\t\t\t   Aliases: ");
			AppendOutputText(*q);
			AppendOutputText("\r\n");
		}

	}

	return TRUE;
}

/* ==== Convert a string representing a service to its port number ==== */
BOOL ServiceToPort(std::vector<char*> v)
{
	servent *sv;
	int temp;		//temporary storage for port number
	
	char portNumber[sizeof(short) + 1] = { '\0' };

	sv = getservbyname(v[0], v[1]);
	if (sv == NULL)
	{
		AppendOutputText("Error in getservbyname\r\n");
		return FALSE;
	}
	
	//convert internet's port number to valid displayable format.
	temp = (short)ntohs(sv->s_port);
	_itoa_s(temp, portNumber, 10);
	
	AppendOutputText("The port number for ");
	AppendOutputText(v[0]);
	AppendOutputText(" is: ");
	AppendOutputText(portNumber);
	AppendOutputText("\r\n");

	WSACleanup();
	
	return TRUE;
}

/* ==== Convert a string representing a port to its service/portocol. ==== */
BOOL PortToService(std::vector<char*> v)
{
	servent *sv;
	int s_port;

	s_port = atoi(v[0]);

	sv = getservbyport(htons(s_port), v[1]);
	if (sv == NULL)
	{
		AppendOutputText("Could not find service in port: ");
		AppendOutputText(v[0]);
		AppendOutputText(".\r\n");
		return FALSE;
	}

	AppendOutputText("The service for ");
	AppendOutputText(v[1]);
	AppendOutputText(" port ");
	AppendOutputText(v[0]);
	AppendOutputText(" is: ");
	AppendOutputText(sv->s_name);
	AppendOutputText("\r\n");

	WSACleanup();
	
	return TRUE;
}