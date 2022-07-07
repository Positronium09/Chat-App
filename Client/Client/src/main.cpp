#include <windows.h>
#include <windowsx.h>

#include "ClientMainWindow.h"
#include "UsernamePromt.h"
#include "WindowClass.h"
#include "Titlebar.h"
#include "utils.h"

#undef string

#pragma comment (lib, "Ws2_32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


std::string ip;
std::string port;
bool saveUsername = true;


void CreateMainWindow(const std::wstring& username);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
	HMODULE msftedit = LoadLibrary(TEXT("Msftedit.dll"));
	if (msftedit == NULL)
	{
		MessageBox(NULL, TEXT("Cannot load Msftedit.dll"), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 1;
	}

	CONFIG config = ParseConfigFile();

	if (config.ip.empty() || config.port.empty())
	{
		MessageBox(NULL, TEXT("Please configure the config.cfg file and restart"), TEXT("Config file error"), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}

	#ifdef _DEBUG
	ip = "127.0.0.1";
	port = "19977";
	#else
	ip = config.ip;
	port = config.port;
	#endif

	Initialize();

	TitleBarLibrary::InitializeTitleBarLibrary();

	WindowWrapper::WindowClass usernamePromtClass{ TEXT("UsernamePromtClassName"), CS_HREDRAW | CS_VREDRAW };

	UsernamePromt promt{ CreateMainWindow };

	if (config.username.empty())
	{
		WindowWrapper::WindowBase::Create(&promt, usernamePromtClass, TEXT("Enter Username"),
			POINT{ CW_USEDEFAULT, CW_USEDEFAULT }, SIZE{ 300, 300 }, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX);

		ShowWindow(promt, SW_SHOW);
		UpdateWindow(promt);
	}
	else
	{
		saveUsername = false;
		CreateMainWindow(config.username);
	}

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


void CreateMainWindow(const std::wstring& username)
{
	if (saveUsername)
	{
		SaveUsernameToConfigFile(username);
	}

	static WindowWrapper::WindowClass mainWindowClass{ TEXT("ClientWindowClassName"), CS_HREDRAW | CS_VREDRAW };

	static ClientMainWindow mainWindow{ username, ip.c_str(), port.c_str() };
	WindowWrapper::WindowBase::Create(&mainWindow, mainWindowClass, username.c_str(),
		POINT{ CW_USEDEFAULT, CW_USEDEFAULT }, SIZE{ 800, 600 }, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX);

	ShowWindow(mainWindow, SW_SHOW);
	UpdateWindow(mainWindow);
}
