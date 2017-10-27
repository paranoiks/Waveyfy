#include "stdafx.h"

ClientSocketRAII::ClientSocketRAII()
{

}

ClientSocketRAII::ClientSocketRAII(SOCKET socket)
{
	_clientSocket = std::make_shared<ClientSocket>(socket);
}

ClientSocket* ClientSocketRAII::GetPointer()
{
	return _clientSocket.get();
}

ClientSocketRAII::~ClientSocketRAII()
{
	
}
