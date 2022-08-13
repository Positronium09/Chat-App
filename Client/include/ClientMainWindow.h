#pragma once

#include "MainWindow.h"
#include "Titlebar.h"
#include "Client.h"
#include "windowMessages.h"

#include <chrono>


class ClientMainWindow : public MainWindow
{
	private:
	Client client;
	std::chrono::steady_clock::time_point start{ };

	void Send();

	LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnMessagePack(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnDisconnected(HWND hWnd, WPARAM wParam, LPARAM lParam);

	public:
	ClientMainWindow(std::wstring username, PCSTR ip, PCSTR port);

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
			REGISTER_HANDLER(MM_MESSAGE_PACK, OnMessagePack)
			REGISTER_HANDLER(MM_DISCONNECTED, OnDisconnected)
		END_MAP()
	END_PROC()
};
