#pragma once


#include "header.h"

#include <set>
#include <vector>
#include <mutex>
#include <string>

#include <windows.h>
#include <winsock2.h>

typedef struct _client
{
	std::wstring username;
	SOCKET clientSocket;
} CLIENT;

inline bool operator<(const CLIENT& lhs, const CLIENT& rhs)
{
	return lhs.clientSocket < rhs.clientSocket;
}

class Server
{
	private:
	HWND hWnd = NULL;
	std::set<CLIENT> connectedClients;
	std::vector<std::thread> listenThreads;
	SOCKET listenSocket = INVALID_SOCKET;
	std::uint64_t clientCount = 0;

	std::thread acceptConnectionsThread;

	std::mutex wcoutMutex{ };
	std::mutex clientsMutex{ };
	std::mutex listenThreadsMutex{ };
	std::mutex broadCastMutex{ };
	std::mutex clientCountMutex{ };

	void AcceptConnections();
	void ListenMessages(CLIENT client);

	void IncreaseClientCount();
	void DecreaseClientCount();

	public:
	Server(Server&) = delete;
	Server& operator=(Server&) = delete;

	Server(PCSTR ip, PCSTR port);
	~Server();

	void StartListening();
	void StopServer();

	void BroadCastMessage(SOCKET excludeSocket, HEADER header, const wchar_t* username, const wchar_t* message);

	std::uint64_t GetClientCount();
	void SetHwnd(HWND hWnd);
};
