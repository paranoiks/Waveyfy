// WebServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define HOST_NAME		"localhost"
#define HOST_PORT		"8080"

void ProcessRequest(SOCKET socket)
{
	ClientSocketRAII clientSocket(socket);
	clientSocket.GetPointer()->Process();
}

int main()
{
	SOCKET socket;
	ServerSocketRAII serverSocket(HOST_NAME, HOST_PORT);
	socket = serverSocket.GetPointer()->Accept();

	ProcessRequest(socket);
    return 0;
}

