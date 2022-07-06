#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <string>


int Initialize();

int Cleanup();

#ifdef string
#undef string
#endif

typedef struct _config
{
	std::wstring username{  };
	std::string ip{  };
	std::string port{  };
} CONFIG;

CONFIG ParseConfigFile();
void SaveUsernameToConfigFile(const std::wstring& username);
