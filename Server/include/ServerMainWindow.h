#pragma once


#include "MainWindow.h"
#include "Server.h"


class ServerMainWindow : public MainWindow
{
	private:
	Server server;
	void Send();

	LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnMessagePack(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnClientCountChanged(HWND hWnd, WPARAM wParam, LPARAM lParam);

	public:
	ServerMainWindow(PCSTR ip, PCSTR port);

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
		REGISTER_HANDLER(MM_MESSAGE_PACK, OnMessagePack)
		REGISTER_HANDLER(MM_CLIENT_COUNT_CHANGED, OnClientCountChanged)
	END_MAP()
	END_PROC()
};
