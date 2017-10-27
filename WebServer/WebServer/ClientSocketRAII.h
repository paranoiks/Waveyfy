#pragma once
class ClientSocketRAII
{
public:
	ClientSocketRAII();
	ClientSocketRAII(SOCKET socket);
	ClientSocket* GetPointer();
	~ClientSocketRAII();
private:
	std::shared_ptr<ClientSocket> _clientSocket;
};

