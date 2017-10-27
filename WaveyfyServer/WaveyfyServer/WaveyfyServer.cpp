#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"

#include<thread>
#include "Server.h"


int main()
{
	Server* server = new Server();

	system("pause");
}
