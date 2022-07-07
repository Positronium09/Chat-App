#include "Client.h"

#include "windowMessages.h"

#include <sstream>
#include <iostream>
#include <thread>

#include <ws2tcpip.h>


Client::Client(const std::wstring& p_username, PCSTR ip, PCSTR port) :
	username(p_username)
{
	addrinfo hints{ };
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* info = nullptr;

	if (getaddrinfo(ip, port, &hints, &info))
	{
		return;
	}

	connectionSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (connectionSocket == INVALID_SOCKET)
	{
		freeaddrinfo(info);
		return;
	}

	if (connect(connectionSocket, info->ai_addr, static_cast<int>(info->ai_addrlen)) == SOCKET_ERROR)
	{
		freeaddrinfo(info);
		closesocket(connectionSocket);
		return;
	}

	freeaddrinfo(info);

	listenThread = std::move(std::thread{ &Client::ListenForMessages, this });
}

Client::~Client()
{
	if (isConnected)
	{
		CloseConnection();
	}
	if (listenThread.joinable())
	{
		listenThread.join();
	}
}

void Client::CloseConnection()
{
	HEADER h = ConstructHeader(HeaderType::DISCONNECT, 0, 0);
	send(connectionSocket, (char*)&h, sizeof(HEADER), NULL);

	closesocket(connectionSocket);
	shutdown(connectionSocket, SD_SEND);

	isConnected = false;
}

void Client::ListenForMessages()
{
	while (true)
	{
		char header[sizeof(HEADER)];
		int bytesRecieved = recv(connectionSocket, header, sizeof(HEADER), NULL);
		if (bytesRecieved == SOCKET_ERROR)
		{
			break;
		}

		HEADER h = ParseHeader(header);

		if (h.type == HeaderType::CONNECT)
		{
			isConnected = true;
			//std::wcout << L"[CONNECTED TO SERVER]\n";
			continue;
		}
		if (h.type == HeaderType::DISCONNECT)
		{
			break;
		}
		if (h.type == HeaderType::INVALID)
		{
			continue;
		}

		#ifdef _DEBUG
		if (h.type == HeaderType::RECIEVED)
		{
			MESSAGEPACK* pack = new MESSAGEPACK{ HeaderType::MESSAGE, L"[DEBUG][Message Recieved]\n" };

			// Window releases the pack
			PostMessage(hWnd, MM_MESSAGE_PACK, 0, (LPARAM)pack);
			//std::wcout << L"[Message Recieved]\n";
			continue;
		}
		#endif

		if (h.type == HeaderType::QUERY_USERNAME)
		{
			h = ConstructHeader(HeaderType::QUERY_USERNAME, static_cast<uint32_t>(username.length() + 1), 0);
			send(connectionSocket, (char*)&h, sizeof(HEADER), NULL);

			send(connectionSocket, (char*)username.c_str(), h.usernameLength, NULL);
		}

		else
		{
			wchar_t* username = new wchar_t[h.usernameLength];
			wchar_t* message = new wchar_t[h.messageLength];

			bytesRecieved = recv(connectionSocket, (char*)username, h.usernameLength, NULL);
			if (bytesRecieved == SOCKET_ERROR)
			{
				break;
			}

			bytesRecieved = recv(connectionSocket, (char*)message, h.messageLength, NULL);
			if (bytesRecieved == SOCKET_ERROR)
			{
				break;
			}

			std::wstringstream sstream;

			sstream << L'[' << username << L"]["
				<< h.hours << L":" << h.minutes << L":" << h.seconds << L"]: "
				<< message << '\n';

			MESSAGEPACK* pack = new MESSAGEPACK{ h.type, sstream.str() };

			// Window releases the pack
			PostMessage(hWnd, MM_MESSAGE_PACK, 0, (LPARAM)pack);
		}
	}

	isConnected = false;
	CloseConnection();
	PostMessage(hWnd, MM_DISCONNECTED, NULL, NULL);
}

bool Client::Send(const wchar_t* message)
{
	if (!isConnected)
	{
		return false;
	}

	HEADER h = ConstructHeader(HeaderType::MESSAGE, 
		static_cast<std::uint32_t>(username.size()) + 1, 
		static_cast<std::uint32_t>(std::wcslen(message)) + 1, true);

	int result = send(connectionSocket, (char*)&h, sizeof(HEADER), NULL);
	if (result == SOCKET_ERROR)
	{
		CloseConnection();
		return false;
	}
	
	result = send(connectionSocket, (char*)username.c_str(), h.usernameLength, NULL);
	if (result == SOCKET_ERROR)
	{
		CloseConnection();
		return false;
	}

	result = send(connectionSocket, (char*)message, h.messageLength, NULL);
	if (result == SOCKET_ERROR)
	{
		CloseConnection();
		return false;
	}

	return true;
}

bool Client::IsConnected()
{
	return isConnected;
}

void Client::SetHwnd(HWND p_hWnd)
{
	hWnd = p_hWnd;
}

const std::wstring& Client::GetUsername()
{
	return username;
}
