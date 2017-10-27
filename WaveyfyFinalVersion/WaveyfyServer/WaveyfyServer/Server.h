#pragma once
#include<thread>

#define hostPort "8080"

#define WINSOCK_MAJOR_VERSION 2
#define WINSOCK_MINOR_VERSION 2

#define DEFAULT_BUFLEN 512
class Server
{
public:
	Server();
	~Server();
	void ListenForClient(SOCKET clientSocket, std::shared_ptr<AudioController> audioController);

private:
	std::vector<std::thread> _threads;
};

