#pragma once

#include "header.h"
#include <windows.h>

#include <string>


#define MM_MESSAGE_PACK (WM_USER + 1)
#define MM_DISCONNECTED (WM_USER + 2)


typedef struct _messagePack
{
	HeaderType type;
	std::wstring message;
} MESSAGEPACK;
