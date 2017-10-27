#pragma once
class ServerSocketRAII
{
public:
	ServerSocketRAII();
	ServerSocketRAII(char* hostName, char* hostPort);
	ServerSocket* GetPointer();
	~ServerSocketRAII();
private:
	std::shared_ptr<ServerSocket> _serverSocket;
};

