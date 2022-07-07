#include "ClientMainWindow.h"

#include "systemcolors.h"
#include "header.h"
#include "windowMessages.h"
#include "utils.h"

#include <sstream>
#include <algorithm>
#include <ctime>
#include <richedit.h>
#include <textserv.h>

#define TIMEOUT_TIMER_ID 1

#ifdef _DEBUG
#define TIMEOUT_SECONDS 1
#else
#define TIMEOUT_SECONDS 5
#endif

static constexpr COLORREF terminalGreenColor = RGB(0x13, 0xa1, 0x0e);
static constexpr COLORREF terminalRedColor = RGB(0xc5, 0x0f, 0x1f);


ClientMainWindow::ClientMainWindow(std::wstring username, PCSTR ip, PCSTR port) :
	client{ username, ip, port }
{

}

LRESULT ClientMainWindow::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	client.SetHwnd(hWnd);

	start = std::chrono::high_resolution_clock::now();
	SetTimer(hWnd, TIMEOUT_TIMER_ID, 125, NULL);

	return MainWindow::OnCreate(hWnd, wParam, lParam);
}

void ClientMainWindow::Send()
{
	if (!client.IsConnected())
	{
		return;
	}

	LRESULT length = SendMessage(hWndMessageEnter, WM_GETTEXTLENGTH, NULL, NULL) + 1;

	wchar_t* message = new wchar_t[length];

	SendMessage(hWndMessageEnter, WM_GETTEXT, length, (LPARAM)message);

	DisplayOnScreen(Format(client.GetUsername(), message).c_str());

	client.Send(message);

	delete[] message;

	SendMessage(hWndMessageEnter, WM_SETTEXT, NULL, (LPARAM)TEXT(""));
}

LRESULT ClientMainWindow::OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	client.CloseConnection();
	PostQuitMessage(0);
	return 0;
}

LRESULT ClientMainWindow::OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (client.IsConnected())
	{
		KillTimer(hWnd, TIMEOUT_TIMER_ID);

		DisplayOnScreen(L"[CONNECTED]\n", terminalGreenColor);
		return 0; 
	}

	auto time = std::chrono::high_resolution_clock::now();

	if (std::chrono::duration_cast<std::chrono::seconds>(time - start).count() > TIMEOUT_SECONDS)
	{
		KillTimer(hWnd, TIMEOUT_TIMER_ID);

		DisplayOnScreen(L"[CONNECTION TIMED OUT]", terminalRedColor);

		SendMessage(hWnd, WM_SETTEXT, NULL, (LPARAM)TEXT("Connection timed out"));
		TitleBarLibrary::UpdateNCArea(hWnd);
	}

	return 0;
}

LRESULT ClientMainWindow::OnMessagePack(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MESSAGEPACK* pack = reinterpret_cast<MESSAGEPACK*>(lParam);

	COLORREF color = NO_COLORING;

	if (pack->type == HeaderType::USER_CONNECTED)
	{
		color = terminalGreenColor;
	}
	else if (pack->type == HeaderType::USER_DISCONNECTED)
	{
		color = terminalRedColor;
	}

	DisplayOnScreen(pack->message.c_str(), color);
	if (GetForegroundWindow() != hWnd)
	{
		MessageBeep(MB_OK);
	}

	delete pack;

	return 0;
}

LRESULT ClientMainWindow::OnDisconnected(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	DisplayOnScreen(L"[DISCONNECTED]\n", terminalRedColor);

	SendMessage(hWnd, WM_SETTEXT, NULL, (LPARAM)TEXT("Disconnected"));
	TitleBarLibrary::UpdateNCArea(hWnd);

	return 0;
}
