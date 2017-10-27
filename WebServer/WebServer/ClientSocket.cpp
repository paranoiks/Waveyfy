#include "stdafx.h"

class ClientSocketImplementation
{
public:
	ClientSocketImplementation(SOCKET socket)
	{
		_socket = socket;
	}

	char* Receive(int* byteCount)
	{
		char *readBuffer = new char[1000];
		int bytesReceived = recv(_socket, readBuffer, 1000, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			readBuffer = nullptr;
		}
		*byteCount = bytesReceived;
		return readBuffer;
	}

	int Send(char* buffer, int count)
	{
		int bytesSent = send(_socket, buffer, count, 0);
		return bytesSent;
	}

	void Process()
	{
		// Receive the request
		int byteCount;
		std::unique_ptr<char> readBuffer = std::make_unique<char>(Receive(&byteCount));
		if (readBuffer == nullptr)
		{
			std::cout << "Read from client failed" << std::endl;
			shutdown(_socket, SD_SEND);
			closesocket(_socket);
		}
		// Regardless of whatever request has been received, send back a simple HTML page that just says "Hello"
		char writeBuffer[68] = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 5\n\nHello";
		Send(writeBuffer, 68);
		// Stop any future sends on this socket and cleanup
		shutdown(_socket, SD_SEND);
		closesocket(_socket);
	}

	~ClientSocketImplementation()
	{

	}
private:
	SOCKET _socket;
};

ClientSocket::ClientSocket()
{

}

ClientSocket::ClientSocket(SOCKET socket) : _clientSocketImplementation(new ClientSocketImplementation(socket))
{
	
}

char* ClientSocket::Receive(int* byteCount)
{
	return _clientSocketImplementation->Receive(byteCount);
}

int ClientSocket::Send(char* buffer, int count)
{
	return _clientSocketImplementation->Send(buffer, count);
}

void ClientSocket::Process()
{
	_clientSocketImplementation->Process();
}

ClientSocket::~ClientSocket()
{	
}
