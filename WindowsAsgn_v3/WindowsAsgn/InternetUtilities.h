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

/*---------------------------------------------------------------------------------
--	FUNCTION:		Analyze Input
--
--	DATE:			January 16, 2016
--
--	REVISED:		January 17, 2016 (Tyler Trepanier-Bracken)
--						Function fleshed out in more detail and implements 
--						switching between the four other functions in this file.						
--
--	DESIGNER:		Tyler Trepanier-Bracken
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--
--	INTERFACE:		void AnalyzeInput(char* text, 
--									  int option)
--
--	PARAMTERS:		char* text
--						Text received by the input text field.
--					int option
--						Used for deciding which Internet function to call.
--
--	RETURNS:		void
--
--	NOTES:
--	Receives text input and splits each portion of the input into arguments 
--  where, depending on the option, they get passed into the designated function.
--
--	All success and error messages will be printed onto the output text field.
---------------------------------------------------------------------------------*/
void AnalyzeInput(char* text, int option);


/*---------------------------------------------------------------------------------
--	FUNCTION:		Host To Ip Address
--
--	DATE:			January 17, 2016
--
--	DESIGNER:		Tyler Trepanier-Bracken
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--
--	INTERFACE:		BOOL HostToIpAddress(char* host)
--
--	PARAMTERS:		char* host
--						Host that will be verified.
--
--	RETURNS:		-Returns FALSE on input error and writes an error message to 
--					 the output.
--					-Returns TRUE on success and outputs all available
--					 information.
--
--	NOTES:
--	Converts the "host" string into an usable IP address where it will be
--	displayed inside of the output text field.
--
--	All success and error messages will be printed onto the output text field.
---------------------------------------------------------------------------------*/
BOOL HostToIpAddress(char* host);


/*---------------------------------------------------------------------------------
--	FUNCTION:		Ip Address To Host
--
--	DATE:			January 17, 2016
--
--	REVISED:		January 17, 2016 (Tyler Trepanier-Bracken)
--						Removed unnecessary parameters.
--
--	DESIGNER:		Tyler Trepanier-Bracken
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--
--	INTERFACE:		BOOL IpAddressToHost(char* ip)
--
--	PARAMTERS:		char* ip
--						IP Address that will be verified.
--
--	RETURNS:		-Returns FALSE on input error and writes an error message to
--					 the output.
--					-Returns TRUE on success and outputs all available 
--					 information.
--
--	NOTES:
--	Analyzes the "IP Address" string and displays all domains (and alias if any) 
--	to the output text field.
--
--	All success and error messages will be printed onto the output text field.
---------------------------------------------------------------------------------*/
BOOL IpAddressToHost(char* ip);


/*---------------------------------------------------------------------------------
--	FUNCTION:		Service To Port
--
--	DATE:			January 17, 2016
--
--	REVISED:		January 17, 2016 (Tyler Trepanier-Bracken)
--						Removed unnecessary paramters.
--
--	DESIGNER:		Tyler Trepanier-Bracken
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--
--	INTERFACE:		BOOL ServiceToPort(std::vector<char*> v)
--
--	PARAMTERS:		std::vector<char*> v
--						Vector that contains the arguments for analyzing which
--						service/protocol belongs to which port.
--
--	RETURNS:		-Returns FALSE on input error and writes an error message to
--					 the output.
--					-Returns TRUE on success and outputs all available
--					 information.
--
--	NOTES:
--	Relies on the user input of "<Service> <Protocol>", this function looks at
--	first two char* strings and relays the port number assign to the service and
--	protocol.
--
--	All success and error messages will be printed onto the output text field.
---------------------------------------------------------------------------------*/
BOOL ServiceToPort(std::vector<char*> v);


/*---------------------------------------------------------------------------------
--	FUNCTION:		Port To Service
--
--	DATE:			January 17, 2016
--
--	REVISED:		January 17, 2016 (Tyler Trepanier-Bracken)
--						Removed unnecessary paramters.
--
--	DESIGNER:		Tyler Trepanier-Bracken
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--
--	INTERFACE:		TCHAR* GetTextFromInput()
--
--	PARAMTERS:		std::vector<char*> v
--						Vector that contains the arguments for analyzing which
--						service/protocol belongs to which port.
--
--	NOTES:
--	Relies on the user input of "<Port #> <Protocol>", this function looks at
--	first two char* strings and relays the service assigned to the port number 
--	and protocol.
--
--	All success and error messages will be printed onto the output text field.
---------------------------------------------------------------------------------*/
BOOL PortToService(std::vector<char*> v);