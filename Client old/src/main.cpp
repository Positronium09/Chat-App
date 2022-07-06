#define WIN32_LEAN_AND_MEAN

#include "utils.h"
#include "Client.h"

#include <iostream>
#include <string>

#include <windows.h>
#include <winsock2.h>

#pragma comment (lib, "Ws2_32.lib")

#ifdef _DEBUG
#define IP "127.0.0.1"
#define PORT "6969"
#else
#define IP "167.71.56.116"
#define PORT "22330"
#endif

int main()
{
	Initialize();

	std::wcout << L"Enter username: ";

	std::wstring username;
	std::wcin >> username;
	std::wcin.clear();

	std::wcout << L"Connecting to server...\n\n";

	Client client{ username.c_str(), IP, PORT };

	while (!client.IsConnected());

	std::wcout << L"\nType .exit to exit\n\n";
	std::wcout << L"----------------------------\n";

	while (true)
	{
		std::wstring message;
		
		std::getline(std::wcin, message);

		if (message == L".exit")
		{
			client.CloseConnection();
			break;
		}
		if (message == L"")
		{
			std::wcin.clear();
			continue;
		}

		client.Send(message.c_str());
	}

	while (client.IsConnected());

	Cleanup();

	return 0;
}
