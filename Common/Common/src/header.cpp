#include "header.h"

#include <ctime>


HEADER ParseHeader(char* header)
{
	HEADER h;
	
	h = *reinterpret_cast<HEADER*>(header);

	return h;
}

HEADER ConstructHeader(HeaderType type, std::uint32_t usernameLength, std::uint32_t messageLength, bool initTime)
{
	HEADER header{ type, 
		usernameLength * static_cast<std::uint32_t>(sizeof(wchar_t)), 
		messageLength * static_cast<std::uint32_t>(sizeof(wchar_t)) };

	if (initTime)
	{
		time_t currentTime;
		struct tm localTime;

		time(&currentTime);
		localtime_s(&localTime, &currentTime);

		header.hours = localTime.tm_hour;
		header.minutes = localTime.tm_min;
		header.seconds = localTime.tm_sec;
	}

	return header;
}
