#define _CRT_SECURE_NO_WARNINGS //To use scanf properly

#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string>


#define BUFSIZE 512   //Maximum buffer size for message passing and retrieval
#define CONN_STR "\\\\.\\pipe\\Pipe" //Connection string 
using namespace std;

int main()
{
	HANDLE hPipe;
	string client_msg = "HELLO FROM THE CLIENT"; //characters to be sent to the server
	int numbers = 200; //numbers to be sent to the server
	const char *cMessage = 0;
	int mode = 1; // 1 for sync and 2 for async
	TCHAR  cBuf[BUFSIZE];
	BOOL   isSuccess = FALSE;
	DWORD  cRead, cToWrite, cWritten, cMode; //variables for pipe read / write
	

	// Try to open a named pipe; wait for it, if necessary. 
	while (1)
	{
		hPipe = CreateFile(
			TEXT(CONN_STR),   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

							// Break if the pipe handle is invalid. 

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			printf("Could not open pipe. GLE=%d\n", GetLastError());
			return -1;
		}

		// All pipe instances are busy, so wait for 10 seconds. 

		if (!WaitNamedPipe(TEXT(CONN_STR),10000))
		{
			printf("Could not open pipe: 10 second wait timed out.");
			return -1;
		}
	}

	// The pipe connected; change to message-read mode or nowait mode
	cMode = PIPE_READMODE_MESSAGE; //Default

	printf("Enter a string (max size 512 chars) and then integer number (between 1 & 999)\n");
	printf("Enter a string: ");
	scanf("%[^\t\n]s", cBuf);
	printf("Enter an integer number: ");
	scanf("%d", &numbers);
	printf("Enter mode (1: Sync, 2: Async):");
	scanf("%d", &mode);
	
	if (mode == 1)
	{
		cMode = PIPE_READMODE_MESSAGE; //sync mode
		printf("CLIENT is sendning data in sync mode.\n");
	}
	else
	{
		cMode = PIPE_NOWAIT; //async mode
		printf("CLIENT is sending data in async mode.\n");
	}
	isSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&cMode,  // new pipe mode 
		NULL,     
		NULL);    
	if (!isSuccess)
	{
		printf("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
		return -1;
	}
	//Send message to the server
	client_msg = cBuf;
	cMessage = client_msg.c_str();
	cToWrite = (strlen(cMessage) + 1) * sizeof(TCHAR);
	printf("Sending %d byte message: \"%s\"\n", cToWrite, cMessage);

	isSuccess = WriteFile(
		hPipe,                  // pipe handle 
		cMessage,             // message 
		cToWrite,              // message length 
		&cWritten,             // bytes written 
		NULL);                  // not overlapped 

	if (!isSuccess)
	{
		printf("WriteFile to pipe failed. GLE=%d\n", GetLastError());
		return -1;
	}
	
	//Send a number to the pipe server
	client_msg = to_string(numbers); //converts numbers to string
	cMessage = client_msg.c_str(); //converts string to char array
	cToWrite = (strlen(cMessage) + 1) * sizeof(TCHAR);
	printf("Sending %d byte message to the server: \"%s\"\n", cToWrite, cMessage);

	isSuccess = WriteFile(
		hPipe,                  // pipe handle 
		cMessage,             // message 
		cToWrite,              // message length 
		&cWritten,             // bytes written 
		NULL);                  // not overlapped 

	if (!isSuccess)
	{
		printf("WriteFile to pipe failed. GLE=%d\n", GetLastError());
		return -1;
	}

	printf("String and numbers are sent to the server, receiving reply as follows:\n");
	do
	{
		// Read from the pipe. 
		isSuccess = ReadFile(
			hPipe,    // pipe handle 
			cBuf,    // buffer to receive reply 
			BUFSIZE * sizeof(TCHAR),  // size of buffer 
			&cRead,  // number of bytes read 
			NULL);    // not overlapped 

		if (!isSuccess && GetLastError() != ERROR_MORE_DATA)
			break;

		printf("\"%s\"\n", cBuf);
	
	} while (!isSuccess);  // repeat loop if ERROR_MORE_DATA 

	if (!isSuccess)
	{
		printf("ReadFile from pipe failed. %d\n", GetLastError());
		return -1;
	}

	printf("\nEnd of message, press ENTER to terminate the connection and exit\n");
	_getch();

	CloseHandle(hPipe); //close the pipe handle
	return 0;
}
