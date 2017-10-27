#pragma once

class ClientSocketImplementation;

class ClientSocket
{
public:
	ClientSocket();
	ClientSocket(SOCKET socket);
	void Process();
	char* Receive(int* byteCount);
	int Send(char* buffer, int count);
	~ClientSocket();
private:
	ClientSocketImplementation* _clientSocketImplementation;
};

