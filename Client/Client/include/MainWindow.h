#pragma once

#include "Window.h"
#include "Titlebar.h"
#include "Client.h"
#include "clientMessages.h"

#include <chrono>


class MainWindow : public WindowWrapper::WindowBase
{
	private:
	TitleBarLibrary::TitleBar titleBar{ TitleBarLibrary::TitleBarParams{ } };
	Client client;
	HWND hWndMessages = NULL;
	HWND hWndMessageEnter = NULL;
	bool firstTime = true;
	std::chrono::steady_clock::time_point start{ };

	HWND CreateRichEdit(COLORREF textColor, COLORREF backgroundColor, DWORD styles = 0);

	void DisplayOnScreen(const wchar_t* toPrint, COLORREF textColor = RGB(1, 1, 1));
	void Send();

	LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkGnd(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnMessageRecieved(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnDisconnected(HWND hWnd, WPARAM wParam, LPARAM lParam);

	public:
	MainWindow(std::wstring username, PCSTR ip, PCSTR port);

	BEGIN_PROC()
		LRESULT result = 0;
		if (TitleBarLibrary::TitleBarHandleMessage(hWnd, msg, wParam, lParam, titleBar, result))
		{
			return result;
		}
		BEGIN_MAP()
			REGISTER_HANDLER(WM_CREATE, OnCreate)
			REGISTER_HANDLER(WM_DESTROY, OnDestroy)
			REGISTER_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
			REGISTER_HANDLER(WM_NOTIFY, OnNotify)
			REGISTER_HANDLER(WM_TIMER, OnTimer)
			REGISTER_HANDLER(MM_MESSAGE_RECIEVED, OnMessageRecieved)
			REGISTER_HANDLER(MM_DISCONNECTED, OnDisconnected)
		END_MAP()
	END_PROC()
};
