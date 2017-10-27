#include "stdafx.h"

ServerSocketRAII::ServerSocketRAII()
{
}

ServerSocketRAII::ServerSocketRAII(char* hostName, char* hostPort)
{
	_serverSocket = std::make_shared<ServerSocket>(hostName, hostPort);
}

ServerSocket* ServerSocketRAII::GetPointer()
{
	return _serverSocket.get();
}

ServerSocketRAII::~ServerSocketRAII()
{	
}
