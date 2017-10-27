// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma comment(lib, "ws2_32.lib")
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <ostream>

#include "ServerSocket.h"
#include "ClientSocket.h"
#include "ServerSocketRAII.h"
#include "ClientSocketRAII.h"




// TODO: reference additional headers your program requires here
