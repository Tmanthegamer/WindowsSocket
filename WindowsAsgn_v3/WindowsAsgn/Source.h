/*=====================================================================================
SOURCE FILE:	Source.h

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
					Organized header file into a cleaner format.

DESIGNER:		Tyler Trepanier-Bracken

PROGRAMMER		Tyler Trepanier-Bracken

NOTES:
Header file declaring all the prototypes required by the Source.cpp file. Initially
includes the InternetUtilities.h as well lists all the global variables and MACROS
that are used with this program.
=====================================================================================*/

/*---------------------------------------------------------------------------------
	MACROS:			Menu Macros

	DATE:			January 17th, 2016

	REVISIONS:		January 18th, 2016 (Tyler Trepanier-Bracken)
						Removed unused utlity macros.

	DESIGNER:		Tyler Trepanier-Bracken

	PROGRAMMER:		Tyler Trepanier-Bracken

	PURPOSE:
	These macros are used initially with the CreateWindow function for making 
	EditText and buttons. This assists the WinProc (Window Procedure) detect 
	events that occur with the designated windows and handle those events.
---------------------------------------------------------------------------------*/
#define IDD_EDIT_TEXT	103
#define IDC_CONVERT     104
#define IDC_CLEAR		105
#define IDC_OUTPUT		106

/*---------------------------------------------------------------------------------
	MACROS:			Utility Macros

	DATE:			January 17th, 2016

	REVISIONS:		January 18th, 2016 (Tyler Trepanier-Bracken)
						Removed unused utlity macros.

	DESIGNER:		Tyler Trepanier-Bracken

	PROGRAMMER:		Tyler Trepanier-Bracken

	PURPOSE:
	Utility Macros used to assist the Analyze Input function in determining which
	function to call.
---------------------------------------------------------------------------------*/


#define EOT 0x04
#define ACK 0x06

#define MAXBUF	8192
void updateStatistic(HWND hList, int avg, int sent, int lost, int data, int time);
HWND CreateListView(HWND parent);
void GetTextFromHost();
void GetTextFromPort();
void CreateSocketInformation(SOCKET s);
LPSOCKET_INFORMATION GetSocketInformation(SOCKET s);
void FreeSocketInformation(SOCKET s);

/*---------------------------------------------------------------------------------
--	FUNCTION:		Get Text From Input
--
--	DATE:			January 16, 2016
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
--	RETURNS:		Returns text gathered from the "input" text box.
--
--	NOTES:
--	Relies on the input button being created before this function's operation.
--	
--	Sends a message to the pre-constructed "input" textfield and returns all the
--	text inside of the input.
---------------------------------------------------------------------------------*/
TCHAR* GetTextFromInput();

/*---------------------------------------------------------------------------------
--	FUNCTION:		Set Output Text
--
--	DATE:			January 17, 2016
--
--	DESIGNER:		Tyler Trepanier-Bracken
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--
--	INTERFACE:		void SetOutputText(TCHAR* text, 
									   int len)
--
--	PARAMETERS:		TCHAR* text
--						Pointer to the text that will be display in the output
--						text field.
--					int len
--						Length of the "text" parameter.
--
--	RETURNS:		void
--
--	NOTES:		
--	Sets the output of the text field to the designated "text" specified.
---------------------------------------------------------------------------------*/
void SetOutputText(TCHAR* text, int len);

/*---------------------------------------------------------------------------------
--	FUNCTION:		Append Output Text
--
--	DATE:			January 17, 2016
--
--	REVISIONS		January 17, 2016 (Tyler Trepanier-Bracken)
--						The output properly APPENDS instead of prepending
--						(which was occuring initally) the output in the text
--						field.
--
--	DESIGNER:		Tyler Trepanier-Bracken
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--
--	INTERFACE:		void AppendOutputText(TCHAR* text)
--
--	PARAMETERS:		TCHAR* text
--						Pointer to the text that will be appended to the output
--						text field.
--
--	RETURNS:		void
--
--	NOTES:
--	Adds on additional text to the output text field.
---------------------------------------------------------------------------------*/
void AppendOutputText(TCHAR* text);

/*---------------------------------------------------------------------------------
--	FUNCTION:		WinMain
--
--	DATE:			October 17, 2015
--
--	REVISIONS:		January 17, 2016 (Tyler Trepanier-Bracken)
--						Created the windows using C++ code instead of resources 
--						and transferred all child window creating to the WM_CREATE
--						portion inside of WndProc (Windows Procedure).
--					January 16, 2016 (Tyler Trepanier-Bracken)
--						Repurposed this function from the SkyeTek application that
--						was made previously. In addition, used a combobox dialog 
--						message box to contain all the child windows needed.
--					
--
--	DESIGNER:		Microsoft Corporation (Original Designer)
--
--	PROGRAMMER:		Tyler Trepanier-Bracken (Re-purposed for this application.)
--
--	INTERFACE:		int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--						PWSTR lspszCmdParam, int nCmdShow)
--
--	PARAMETERS:		HINSTANCE hInst
--						Handle to the current instance of the application.
--					HINSTANCE hprevInstance
--						Handle to the previous instance of the application.
--						For a Win32-based application, this parameter is always NULL.
--					PWSTR lspszCmdParam
--						Pointer to a null-terminated string that specifies the command
--						line for the application, excluding the program name.
--					int nCmdShoW
--						Specifies how the window is to be shown.
--
--	RETURNS:		The exit value contained in that message's wParam
--					parameter indicates success, and that the function
--					terminates when it receives a WM_QUIT message. Zero
--					indicates that the function terminates before entering
--					the message loop.
--
--	NOTES:
--	This function is called by the system as the initial entry point for
--	Windows Embedded CE-based applications. Used to create the window
--	where the application will take place.
---------------------------------------------------------------------------------*/
int WINAPI WinMain(	HINSTANCE hInst, 
					HINSTANCE hprevInstance,
					LPSTR lspszCmdParam, 
					int nCmdShow);

/*---------------------------------------------------------------------------------
--	FUNCTION:		Window Procedure
--
--	DATE:			October 17, 2015
--
--	REVISIONS:		January 17, 2016 (Tyler Trepanier-Bracken)
--						-Removed all combo box creation/destruction code and 
--						 instead added the pre-existed child elements to a standard
--						 static sized window.
--						-Added an additional text field for output to be displayed 
--						 upon
--						-Implemented AnalyzeInput function.
--						-Implemented "Clear" button functionality.
--					January 16, 2016 (Tyler Trepanier-Bracken)
--						Modified existing code to create the input text field,
--						list and two buttons (convert / clear) inside of the
--						Combo Box Dialog window.
--						
--
--	DESIGNER:		Aman Abdulla (Original Designer)
--
--	PROGRAMMER:		Tyler Trepanier-Bracken
--						(Re-purposed from SkyeTek RFID Scanner)
--						(Originally re-purposed from Lab: Menu Example #5)
--
--	INTERFACE:		LRESULT CALLBACK WndProc(
--						HWND hwnd,
--						UINT Message,
--						WPARAM wParam,
--						LPARAM lParam)
--
--	PARAMETERS:		HWND hwnd
--						Window handle to designated for this specific application
--					UINT Message
--						Message received from Windows.
--					WPARAM wParam
--						Input received from keyboard.
--					LPARAM lParam
--						Input received from mouse.
--
--	RETURNS:		LRESULT CALLBACK.
--
--	NOTES:
--	Handles the continous stream of messages and input sent from Windows and
--	the user.
--
--	WM_COMMAND
--		IDC_CONVERT:	Perform the AnalyzeInput function on the input text 
							field.
--		IDC_CLEAR:		Clear all text in the output field.
--
--	WM_CREATE:			Creates several child elements to be placed inside of 
						the parent window
--							-input:		Input text field
--							-output:	Output text field
--							-combobox:	List of available conversions
--							-covertBtn: Convert button that converts text input 
										and uses it.
--							-clearBtn:	Clears the output text fields.
--
--	WM_DESTROY
--		Closes the application.
--
---------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, 
						 UINT Msg, 
						 WPARAM wParam, 
						 LPARAM lParam);

