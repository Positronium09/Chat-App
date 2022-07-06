#define WIN32_LEAN_AND_MEAN

#undef UNICODE

#include "Server.h"
#include "utils.h"

#include <iostream>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")


int main()
{
	Initialize();

	Server server{ "127.0.0.1", "19977" };

	server.StartListening();

	Cleanup();

	return 0;
}
