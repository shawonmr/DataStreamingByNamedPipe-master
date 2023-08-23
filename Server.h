#pragma once
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <vector>
#include <iostream>      
#include <thread>         
#include <mutex>         


#define BUFSIZE 512 //Maximum buffer size for message passing and retrieval
#define CONN_STR "\\\\.\\pipe\\Pipe" //Connection string

using namespace std;
std::mutex mtx;           // mutex for critical section

//This is the class that the server registers and stores client's information
class NamedPipeClientData
{
	private:
		int num;
		char buff[BUFSIZE];
	public:
		NamedPipeClientData() {}; //Constructor
		~NamedPipeClientData() {}; //Destructor
		NamedPipeClientData(int n, char b[])
		{
			num = n;
			for (int i = 0; i < BUFSIZE; i++)
				buff[i] = b[i];
			
		}
		void set_num(int n) //set method for the number
		{
			num = n;
		}
		int get_num()    //get method for the number
		{
			return num;
		}
		void set_buff(char b[])  //set method for the buffer
		{
			for (int i = 0; i < BUFSIZE; i++)
				buff[i] = b[i];
		}
		char *get_buff() //get method for the buffer
		{
			return buff;
		}

};