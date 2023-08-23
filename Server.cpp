#include "Server.h"

//vector of NamedPipeClientData class type that stores all the objects 
static vector<NamedPipeClientData> vectorForClientData;
//thread function for each thread that handles each client
DWORD WINAPI serverThreadForClient(LPVOID); 
//receives and processes client's message
void processAnswerToRequest(LPTSTR, LPTSTR, LPDWORD, int &msg_count, 
	static vector<NamedPipeClientData> &vectorForClientData, NamedPipeClientData &ob); 


int main()
{
	BOOL   isConnected = FALSE;
	DWORD  sThreadId = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	int i = 0;
	

	// The main loop creates an instance of the named pipe and 
	// then waits for a client on a thread to connect to it. 

	printf("Server waiting for client connection on : %s\n\n", CONN_STR);
	for (;;)
	{
		
		
		hPipe = CreateNamedPipe(
			TEXT(CONN_STR),            // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                 

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe failed, %d\n", GetLastError());
			return -1;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value otherwise error values.

		isConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (isConnected) //connection is successful
		{
			printf("Client connected, creating a processing thread.\n");

			// Create a thread for this client. 
			hThread = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				serverThreadForClient,    // thread proc
				(LPVOID)hPipe,    // thread parameter 
				0,                 // not suspended 
				&sThreadId);      // returns thread ID 

			if (hThread == NULL)
			{
				printf("CreateThread failed, %d\n", GetLastError());
				return -1;
			}
			else CloseHandle(hThread);
			
			
		}
		else
			// The client could not connect, so close the pipe. 
			CloseHandle(hPipe);
	}
	
	
	return 0;
}

// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop.
DWORD WINAPI serverThreadForClient(LPVOID lpvParam)
{
	HANDLE hHeap = GetProcessHeap();
	//buffer for request received
	TCHAR* sRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
	//buffer for reply
	TCHAR* sReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));

	DWORD sBytesRead = 0, sReplyBytes = 0, sWritten = 0;
	BOOL isSuccess = FALSE;
	HANDLE hPipe = NULL;
	

	// Print thread creation messages. 
	printf("serverThreadForClient created, receiving and processing messages.\n");
	// The thread's parameter is a handle to a pipe object instance. 
	hPipe = (HANDLE)lpvParam;
	// Loop until done reading
	int msg_count = 0; //counts the no of messages received from the client
	//Object of NamedPipeClientData class; server creates it for client's access
	NamedPipeClientData ob; 
	while (1)
	{
		// Read client requests from the pipe. 
		isSuccess = ReadFile(
			hPipe,        // handle to pipe 
			sRequest,    // buffer to receive data 
			BUFSIZE * sizeof(TCHAR), // size of buffer 
			&sBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		if (!isSuccess || sBytesRead == 0)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				
				printf("serverThreadForClient: client disconnected.\n");

			}
			else
			{
				
				printf("serverThreadForClient ReadFile failed, %d\n", GetLastError());

			}
			
					
			break;
		}

		// Process the incoming message.
		processAnswerToRequest(sRequest, sReply, &sReplyBytes,msg_count, vectorForClientData, ob);
		// Write the reply to the pipe. 
		isSuccess = WriteFile(
			hPipe,        // handle to pipe 
			sReply,     // buffer to write from 
			sReplyBytes, // number of bytes to write 
			&sWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		if (!isSuccess || sReplyBytes != sWritten)
		{
			printf("serverThreadForClient WriteFile failed, %d\n", GetLastError());
			
			break;
		}
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. 

	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	

	printf("\nClients that called\n");
	for (int i = 0; i < vectorForClientData.size(); i++)
	{
		printf("%d, %s\n", vectorForClientData[i].get_num(), vectorForClientData[i].get_buff());
	}

	printf("serverThreadForClient number %s exiting.\n\n", sRequest);
	
	//free the allocated heap
	HeapFree(hHeap, 0, sRequest);
	HeapFree(hHeap, 0, sReply);

	return 1;
}

// This routine is a simple function to print the client request to the console,
// store it to a vector and populate the reply buffer with a default data string. 
void processAnswerToRequest(LPTSTR sRequest,
	LPTSTR sReply,
	LPDWORD sBytes,
	int &msg_count,
	static vector<NamedPipeClientData> &vectorForClientData,
	NamedPipeClientData &ob)
{
	
	printf("Client Requested Message:\"%s\"\n", sRequest);
	int i;
	
	msg_count++; //increases message count each time client sends a message
	int n;
	char b[BUFSIZE];
	//printf("msg_count: %d\n", msg_count);
	if (msg_count == 1)
	{
		for (i = 0; i < BUFSIZE; i++)
		{
			b[i] = sRequest[i]; //copy the received chars
		}
		ob.set_buff(b); //sets the private member char array variable
	}
	if (msg_count == 2)
	{
		n = atoi(sRequest);   //convert the received chars to number
		msg_count = 0; // resets message count variable
	}
	
	ob.set_num(n); //sets the private int variable
	// critical section 
	mtx.lock(); 
	if(n > 0 && n < 1000)  //Only stores clients messages that numbers belong to range[1  to 999]
		vectorForClientData.push_back(ob); //store the information in a NamedPipeClientData vector
	mtx.unlock(); //releases mutex
	
	// Send message back to the client.
	StringCchCopy(sReply, BUFSIZE, TEXT("SERVER: Done with the client message processing."));
	*sBytes = (lstrlen(sReply) + 1) * sizeof(TCHAR);
}