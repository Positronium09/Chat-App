#pragma once

#include <cstdint>


enum class HeaderType : std::uint32_t
{
	INVALID = 0,
	MESSAGE = 1,
	CONNECT = 2,
	DISCONNECT = 3,
	RECIEVED = 4,
	BROADCAST = 5,
	QUERY_USERNAME = 6,
	USER_DISCONNECTED = 7
};


typedef struct _header
{
	HeaderType type = HeaderType::INVALID;
	std::uint32_t usernameLength = 0;
	std::uint32_t messageLength = 0;
	int hours{ };
	int minutes{ };
	int seconds{ };
} HEADER;


HEADER ParseHeader(char* header);
HEADER ConstructHeader(HeaderType type, std::uint32_t usernameLength, std::uint32_t messageLength, bool initTime=false);
