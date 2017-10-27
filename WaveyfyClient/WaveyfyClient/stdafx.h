// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma comment(lib, "ws2_32.lib")
#include "targetver.h"

// Windows Header Files:
#include <stdio.h>
#include <tchar.h>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <ostream>

#include<vector>

#include <dsound.h>

#include <windows.h>
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#define hostPort "8080"
#define WINSOCK_MAJOR_VERSION 2
#define WINSOCK_MINOR_VERSION 2
#define DEFAULT_BUFLEN 1024


// TODO: reference additional headers your program requires here
