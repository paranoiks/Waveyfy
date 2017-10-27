#pragma once

#define WINSOCK_MINOR_VERSION	2
#define WINSOCK_MAJOR_VERSION	2

class ServerSocketImplementation;

class ServerSocket
{
public:
	ServerSocket();
	ServerSocket(char* hostName,  char* hostPort);
	SOCKET Accept(void);
	~ServerSocket();
private:	
	ServerSocketImplementation* _serverSocketImplementation;
};

