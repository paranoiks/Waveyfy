#include "stdafx.h"

class ServerSocketImplementation
{
public:
	ServerSocketImplementation(char* hostName, char* hostPort)
	{
		WSADATA wsaData;
		int status;
		struct addrinfo * data;
		struct addrinfo hints;

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
		if (getaddrinfo(hostName, hostPort, &hints, &data))
		{
			std::cout << "Unable to translate host name to address" << std::endl;
			WSACleanup();
		}
		// Create the server socket
		_listenSocket = socket(data->ai_family, data->ai_socktype, data->ai_protocol);
		if (_listenSocket == INVALID_SOCKET)
		{
			std::cout << "Unable to create server socket" << std::endl;
			freeaddrinfo(data);
			WSACleanup();
		}
		// Setup for listening on this socket
		status = bind(_listenSocket, data->ai_addr, static_cast<int>(data->ai_addrlen));
		if (status == SOCKET_ERROR)
		{
			closesocket(_listenSocket);
			freeaddrinfo(data);
			WSACleanup();
		}
		freeaddrinfo(data);
		// Start listening
		status = listen(_listenSocket, SOMAXCONN);
		if (status == SOCKET_ERROR)
		{
			closesocket(_listenSocket);
			WSACleanup();
		}
	}

	SOCKET Accept()
	{
		_clientSocket = accept(_listenSocket, nullptr, nullptr);
		closesocket(_listenSocket);

		return _clientSocket;
	}

	~ServerSocketImplementation()
	{		
		closesocket(_clientSocket);
		closesocket(_listenSocket);
	}
private:
	SOCKET _clientSocket;
	SOCKET _listenSocket;
};

ServerSocket::ServerSocket()
{

}

ServerSocket::ServerSocket(char* hostName, char* hostPort) : _serverSocketImplementation(new ServerSocketImplementation(hostName, hostPort))
{
	
}

SOCKET ServerSocket::Accept(void)
{
	return _serverSocketImplementation->Accept();
}

ServerSocket::~ServerSocket()
{	
	_serverSocketImplementation->~ServerSocketImplementation();
}
