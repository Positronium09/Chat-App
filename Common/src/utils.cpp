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


std::string ConvertToString(const std::wstring& text)
{
	if (text.empty())
	{
		return "";
	}

	int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()) + 1, nullptr, 0, nullptr, nullptr);

	char* converted = new char[size];
	WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()) + 1, converted, size, nullptr, nullptr);

	std::string str = converted;
	delete[] converted;

	return str;
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
	
	std::wifstream file{ filepath };

	std::wstring line;
	while (std::getline(file, line))
	{
		auto offset = line.find(L"=", 0);
		auto key = line.substr(0, offset);
		auto value = line.substr(offset + 1);

		if (key == L"username")
		{
			config.username = value;
		}
		else if (key == L"ip")
		{
			config.ip = ConvertToString(value);
		}
		else if (key == L"port")
		{
			config.port = ConvertToString(value);
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
		stringstream << L"username=" << username << '\n';

	}
	else
	{
		offset += 9;
		stringstream << username;
	}

	std::wstring insertString = stringstream.str();

	fileContents.insert(offset, insertString);

	file.seekg(0, std::ios_base::beg);
	file << fileContents;
}
