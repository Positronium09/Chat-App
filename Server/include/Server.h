#pragma once

#define WIN32_LEAN_AND_MEAN


#include "header.h"

#include <set>
#include <vector>
#include <mutex>

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
	std::set<CLIENT> connectedClients;
	std::vector<std::thread> listenThreads;
	SOCKET listenSocket = INVALID_SOCKET;
	std::uint64_t clientCount = 0;

	std::mutex wcoutMutex{ };
	std::mutex clientsMutex{ };
	std::mutex listenThreadsMutex{ };
	std::mutex broadCastMutex{ };
	std::mutex clientCountMutex{ };

	void AcceptConnections();
	void ListenMessages(CLIENT client);
	void BroadCastMessage(SOCKET excludeSocket, HEADER header, const wchar_t* username, const wchar_t* message);

	void IncreaseClientCount();
	void DecreaseClientCount();

	public:
	Server(Server&) = delete;
	Server& operator=(Server&) = delete;

	Server(PCSTR ip, PCSTR port);
	~Server();

	void StartListening();
	std::uint64_t GetClientCount();
};
