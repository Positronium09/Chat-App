#include "ServerMainWindow.h"
#include "WindowClass.h"
#include "Window.h"
#include "utils.h"

#include <iostream>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
	HMODULE msftedit = LoadLibrary(TEXT("Msftedit.dll"));
	if (msftedit == NULL)
	{
		MessageBox(NULL, TEXT("Cannot load Msftedit.dll"), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 1;
	}

	TitleBarLibrary::InitializeTitleBarLibrary();

	Initialize();

	WindowWrapper::WindowClass mainWindowClass{ TEXT("ServerWindowClassName"), CS_HREDRAW | CS_VREDRAW };

	ServerMainWindow mainWindow{ "127.0.0.1", "19977" };
	WindowWrapper::WindowBase::Create(&mainWindow, mainWindowClass, L"SERVER | 0 CLIENTS CONNECTED",
		POINT{ CW_USEDEFAULT, CW_USEDEFAULT }, SIZE{ 800, 600 }, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX);

	ShowWindow(mainWindow, SW_SHOW);
	UpdateWindow(mainWindow);

	MSG msg{ };
	while (GetMessage(&msg, NULL, NULL, NULL) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Cleanup();

	FreeLibrary(msftedit);

	return static_cast<int>(msg.wParam);
}
