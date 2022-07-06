#pragma once

#include <string>
#include <mutex>

#include <windows.h>
#include <winsock2.h>


class Client
{
	private:
	HWND hWnd = NULL;
	std::wstring username;
	bool isConnected = false;
	SOCKET connectionSocket = INVALID_SOCKET;
	std::mutex socketMutex{ };
	std::thread listenThread;

	void ListenForMessages();

	public:
	Client(Client&) = delete;
	Client& operator=(Client&) = delete;

	Client(const std::wstring& username, PCSTR ip, PCSTR port);
	~Client();

	bool Send(const wchar_t* message);
	void CloseConnection();
	bool IsConnected();
	void SetHwnd(HWND hWnd);
	const std::wstring& GetUsername();
};

typedef struct _messagePack
{
	std::wstring message;
} MESSAGEPACK;
