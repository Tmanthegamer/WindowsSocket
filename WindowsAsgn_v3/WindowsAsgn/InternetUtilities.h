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
Header file declaring all the prototypes required by the InternetUtilities.cpp file. 
Includes all the shared include files necessary for this program. The purpose of this
header file is to contain all the utilities necessary to parse the text input received
by the User and assign them to their assigned function based on the user's preference.

All errors will be displayed on the output text field.
=====================================================================================*/

#include <windows.h>
#include <Windowsx.h>
#include <vector>
#include <string>
