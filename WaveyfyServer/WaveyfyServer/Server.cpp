#include "stdafx.h"
#include "Server.h"
#include <fstream>
#include <iostream>
#include <string>


Server::Server()
{	
	std::string hostAddressString;
	std::ifstream myfile("ip.txt");
	if (myfile.is_open())
	{
		while (getline(myfile, hostAddressString))
		{
		}
		myfile.close();
	}

	WSADATA wsaData;
	int status;
	struct addrinfo * data;
	struct addrinfo hints;

	int iResult;
	struct addrinfo *result = NULL;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	status = WSAStartup(MAKEWORD(WINSOCK_MAJOR_VERSION, WINSOCK_MINOR_VERSION), &wsaData);
	if (status != 0)
	{
		std::cout << "Unable to start sockets layer" << std::endl;
	}
	// Resolve the server address and port
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(hostAddressString.c_str(), hostPort, &hints, &data))
	{
		std::cout << "Unable to translate host name to address" << std::endl;
		WSACleanup();
	}
	// Create the server socket
	SOCKET listenSocket = socket(data->ai_family, data->ai_socktype, data->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "Unable to create server socket" << std::endl;
		freeaddrinfo(data);
		WSACleanup();
	}
	// Setup for listening on this socket
	status = bind(listenSocket, data->ai_addr, static_cast<int>(data->ai_addrlen));
	if (status == SOCKET_ERROR)
	{
		closesocket(listenSocket);
		freeaddrinfo(data);
		WSACleanup();
	}
	freeaddrinfo(data);
	// Start listening
	status = listen(listenSocket, SOMAXCONN);

	//initialise our audio controller
	std::shared_ptr<AudioController> audioController = std::shared_ptr<AudioController>(new AudioController());

	//accept connections from the clients and put them on a new thread every time
	while (true)
	{
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		_threads.push_back(std::thread(&Server::ListenForClient, this, clientSocket, audioController));
		listen(listenSocket, SOMAXCONN);
	}

	for (int i = 0; i < _threads.size(); i++)
	{
		_threads[i].join();
	}
}

/// <summary>
/// Create a new isntance of the client class
/// This method is always ran on a new thread
/// </summary>
void Server::ListenForClient(SOCKET clientSocket, std::shared_ptr<AudioController> audioController)
{
	Client* client = new Client(clientSocket, (std::shared_ptr<Server>)(this), audioController);
}

Server::~Server()
{
}


