#include "include/utils.h"

#include <sstream>
#include <fstream>
#include <winsock2.h>
#include <stdio.h>
#include <strsafe.h>
#include <pathcch.h>
#include <fcntl.h>
#include <io.h>

#pragma comment (lib, "Pathcch.lib")


static const wchar_t* configFilename = L"\\config.cfg";


int Initialize()
{
	(void)_setmode(_fileno(stdout), _O_U16TEXT);
	(void)_setmode(_fileno(stdin), _O_U16TEXT);

	HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(outputHandle, &mode);
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(outputHandle, mode);

	WSADATA wsaData{ };
	if (int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		result != 0)
	{
		return 1;
	}

	return 0;
}

int Cleanup()
{
	WSACleanup();

	return 0;
}

bool FileExists(const wchar_t* path)
{
	DWORD attributes = GetFileAttributes(path);

	return !(attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY);
}

CONFIG ParseConfigFile()
{
	CONFIG config{ };

	wchar_t filepath[MAX_PATH];

	GetModuleFileNameW(NULL, filepath, MAX_PATH);
	PathCchRemoveFileSpec(filepath, MAX_PATH);
	StringCchCatW(filepath, MAX_PATH, configFilename);

	if (!FileExists(filepath))
	{
		HANDLE fileHandle = CreateFileW(filepath, GENERIC_WRITE, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		DWORD bytesWritten = 0;
		char toWrite[] = "username=\nip=\nport=\n";
		WriteFile(fileHandle, toWrite, static_cast<DWORD>(std::strlen(toWrite)), &bytesWritten, nullptr);

		CloseHandle(fileHandle);
		return config;
	}
	
	std::ifstream file{ filepath };

	std::string line;
	while (std::getline(file, line))
	{
		auto offset = line.find("=", 0);
		auto key = line.substr(0, offset);
		auto value = line.substr(offset + 1);

		if (key == "username")
		{
			int len = (int)strlen(value.c_str());
			int strLen = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), len, NULL, 0);
			WCHAR* username = new WCHAR[strLen + 1llu];
			if (username)
			{
				MultiByteToWideChar(CP_UTF8, 0, value.c_str(), len, username, strLen);
				username[strLen] = L'\0';
				config.username = username;
				delete[] username;
			}
			
		}
		else if (key == "ip")
		{
			config.ip = value;
		}
		else if (key == "port")
		{
			config.port = value;
		}
	}

	return config;
}

void SaveUsernameToConfigFile(const std::wstring& username)
{
	wchar_t filepath[MAX_PATH];

	GetModuleFileNameW(NULL, filepath, MAX_PATH);
	PathCchRemoveFileSpec(filepath, MAX_PATH);
	StringCchCatW(filepath, MAX_PATH, configFilename);

	std::wfstream file{ filepath, std::ios::in | std::ios::out };

	std::wstringstream stringstream;
	stringstream << file.rdbuf();

	std::wstring fileContents = stringstream.str();

	auto offset = fileContents.find(L"username=", 0);

	stringstream.str(L"");

	if (offset == std::wstring::npos)
	{
		offset = 0;
		int len = (int)wcslen(username.c_str());
		int strLen = WideCharToMultiByte(CP_UTF8, 0, username.c_str(), len, NULL, 0, NULL, NULL);
		CHAR* str = new CHAR[strLen + 1llu];
		if (str != nullptr)
		{
			WideCharToMultiByte(CP_UTF8, 0, username.c_str(), len, str, strLen, NULL, NULL);
			str[strLen] = '\0';
			stringstream << str;
			delete[] str;
		}
		stringstream << L"username=" << username << '\n';

	}
	else
	{
		offset += 9;
		int len = (int)wcslen(username.c_str());
		int strLen = WideCharToMultiByte(CP_UTF8, 0, username.c_str(), len, NULL, 0, NULL, NULL);
		CHAR* str = new CHAR[strLen + 1llu];
		if (str != nullptr)
		{
			WideCharToMultiByte(CP_UTF8, 0, username.c_str(), len, str, strLen, NULL, NULL);
			str[strLen] = '\0';
			stringstream << str;
			delete[] str;
		}
	}

	std::wstring insertString = stringstream.str();

	fileContents.insert(offset, insertString);

	file.seekg(0, std::ios_base::beg);
	file << fileContents;
}

std::wstring Format(const std::wstring& username, const wchar_t* message)
{
	std::wstringstream stream;

	time_t currentTime;
	struct tm localTime;

	time(&currentTime);
	localtime_s(&localTime, &currentTime);

	const int hours = localTime.tm_hour;
	const int minutes = localTime.tm_min;
	const int seconds = localTime.tm_sec;

	stream << L'[' << username << L"]["
		<< hours << L":" << minutes << L":" << seconds << L"]: "
		<< message << '\n';

	return stream.str();
}