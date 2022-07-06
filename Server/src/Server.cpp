#include "Server.h"

#include "header.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <memory>

#include <ws2tcpip.h>


Server::Server(PCSTR ip, PCSTR port)
{
	addrinfo hints{ };
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* info = nullptr;
	getaddrinfo(ip, port, &hints, &info);

	listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		freeaddrinfo(info);
		return;
	}

	if (bind(listenSocket, info->ai_addr, static_cast<int>(info->ai_addrlen)) == SOCKET_ERROR)
	{
		freeaddrinfo(info);
		closesocket(listenSocket);
		return;
	}

	freeaddrinfo(info);
}

Server::~Server()
{
	shutdown(listenSocket, SD_BOTH);
}


void Server::ListenMessages(CLIENT client)
{
	const auto closeConnection = [client]()
	{
		auto sendHeader = ConstructHeader(HeaderType::DISCONNECT, 0, 0);
		send(client.clientSocket, (char*)&sendHeader, sizeof(sendHeader), NULL);

		closesocket(client.clientSocket);
		shutdown(client.clientSocket, SD_RECEIVE);
	};

	while (true)
	{
		HEADER header{ };
		int bytesRecieved = recv(client.clientSocket, (char*)&header, sizeof(HEADER), NULL);
		if (bytesRecieved == SOCKET_ERROR)
		{
			break;
		}

		if (header.type == HeaderType::DISCONNECT)
		{
			break;
		}
		if (header.type == HeaderType::INVALID)
		{
			continue;
		}

		if (header.type == HeaderType::MESSAGE)
		{
			broadCastMutex.lock();

			wchar_t* username = new wchar_t[header.usernameLength];
			wchar_t* message = new wchar_t[header.messageLength];

			bytesRecieved = recv(client.clientSocket, (char*)username, header.usernameLength, NULL);
			if (bytesRecieved == SOCKET_ERROR)
			{
				break;
			}

			bytesRecieved = recv(client.clientSocket, (char*)message, header.messageLength, NULL);
			if (bytesRecieved == SOCKET_ERROR)
			{
				break;
			}

			#ifdef _DEBUG
			HEADER recievedHeader = ConstructHeader(HeaderType::RECIEVED, 0, 0);
			send(client.clientSocket, (char*)&recievedHeader, sizeof(HEADER), NULL);
			#endif

			broadCastMutex.unlock();

			header.type = HeaderType::BROADCAST;
			BroadCastMessage(client.clientSocket, header, username, message);


			std::lock_guard lock{ wcoutMutex };

			std::wcout << L'[' << username << L"]["
				<< header.hours << L":" << header.minutes << L":" << header.seconds << L"]: "
				<< message << '\n';

			delete[] username;
			delete[] message;
		}
	}

	std::lock_guard wcoutLock{ wcoutMutex };
	std::lock_guard clientsLock{ clientsMutex };

	std::wstringstream stringstream{ };
	stringstream << L'[' << client.username << L']' << L"[CLIENT DISCONNECTED]";
	std::wstring message = stringstream.str();

	std::wcout << L"\033[31m" << message << L"\033[0m\n";

	HEADER header = ConstructHeader(HeaderType::USER_DISCONNECTED, 
		static_cast<uint32_t>(7), 
		static_cast<uint32_t>(message.length() + 1), true);

	BroadCastMessage(client.clientSocket, header, L"SERVER", message.c_str());

	closeConnection();
	DecreaseClientCount();

	connectedClients.erase(client);
}

void Server::BroadCastMessage(SOCKET excludeSocket, HEADER header, const wchar_t* username, const wchar_t* message)
{
	std::lock_guard lock{ broadCastMutex };
	
	for (auto& client : connectedClients)
	{
		if (client.clientSocket == excludeSocket)
		{
			continue;
		}

		int result = send(client.clientSocket, (char*)&header, sizeof(HEADER), NULL);

		if (result != SOCKET_ERROR)
		{
			result = send(client.clientSocket, (char*)username, header.usernameLength, NULL);

			result = send(client.clientSocket, (char*)message, header.messageLength, NULL);
		}
	}
}

void Server::IncreaseClientCount()
{
	std::lock_guard lock{ clientCountMutex };
	clientCount++;
	SetConsoleTitleW((std::to_wstring(clientCount) + L" Clients connected").c_str());
}

void Server::DecreaseClientCount()
{
	std::lock_guard lock{ clientCountMutex };
	clientCount--;
	SetConsoleTitleW((std::to_wstring(clientCount) + L" Clients connected").c_str());
}

void Server::AcceptConnections()
{
	while (true)
	{
		SOCKET clientSocket = WSAAccept(listenSocket, NULL, NULL, NULL, NULL);

		if (clientSocket == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAEINTR)
			{
				return;
			}
			continue;
		}

		HEADER header = ConstructHeader(HeaderType::CONNECT, 0, 0);
		send(clientSocket, (char*)&header, sizeof(HEADER), NULL);

		header.type = HeaderType::QUERY_USERNAME;
		send(clientSocket, (char*)&header, sizeof(HEADER), NULL);

		recv(clientSocket, (char*)&header, sizeof(HEADER), NULL);
		wchar_t* username = new wchar_t[header.usernameLength];

		recv(clientSocket, (char*)username, header.usernameLength, NULL);

		CLIENT client{ username, clientSocket };

		delete[] username;

		{
			std::lock_guard lock{ clientsMutex };
			connectedClients.insert(client);
		}

		std::thread t{ &Server::ListenMessages, this, client };

		std::lock_guard lock{ listenThreadsMutex };
		listenThreads.push_back(std::move(t));
		IncreaseClientCount();

		std::lock_guard wcoutLock{ wcoutMutex };
		std::wcout << L"\033[32m" << L'[' << client.username << L']' << L"[CLIENT CONNECTED]" << L"\033[0m\n";
	}
}

void Server::StartListening()
{
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(listenSocket);
		return;
	}
	std::thread acceptConnectionsThread{ &Server::AcceptConnections, this };

	HWND console = GetConsoleWindow();

	const auto disconnectClients = [this]()
	{
		std::lock_guard lock{ clientsMutex };
		for (auto& client : connectedClients)
		{
			auto sendHeader = ConstructHeader(HeaderType::DISCONNECT, 0, 0);
			send(client.clientSocket, (char*)&sendHeader, sizeof(sendHeader), NULL);

			closesocket(client.clientSocket);
		}
	};

	const auto stopThreads = [this]()
	{
		std::lock_guard lock{ listenThreadsMutex };
		for (auto& t : listenThreads)
		{
			if (t.joinable())
			{
				t.join();
			}
		}
	};

	while (true)
	{
		if (GetAsyncKeyState(VK_ESCAPE) & 1 && GetForegroundWindow() == console)
		{
			int result = closesocket(listenSocket);

			acceptConnectionsThread.join();

			disconnectClients();
			
			stopThreads();

			break;
		}
	}
}

std::uint64_t Server::GetClientCount()
{
	std::lock_guard lock{ clientCountMutex };
	return clientCount;
}
