#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <mutex>

#include <windows.h>
#include <winsock2.h>


class Client
{
	private:
	std::wstring username;
	bool isConnected = false;
	SOCKET connectionSocket = INVALID_SOCKET;
	std::mutex socketMutex{ };
	std::thread listenThread;

	void ListenForMessages();

	public:
	Client(std::wstring username, PCSTR ip, PCSTR port);
	~Client();

	bool Send(const wchar_t* message);
	void CloseConnection();
	bool IsConnected();
};
