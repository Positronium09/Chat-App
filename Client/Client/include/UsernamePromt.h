#pragma once

#include "MainWindow.h"
#include "Window.h"
#include "Titlebar.h"

#include <functional>


class UsernamePromt : public WindowWrapper::WindowBase
{
	private:
	TitleBarLibrary::TitleBar titleBar{ TitleBarLibrary::TitleBarParams{ } };

	std::function<void(const std::wstring&)> function;

	HWND hWndEdit = NULL;
	bool firstTime = true;
	bool userClicked = false;

	void CallCallback();

	LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);

	public:
	UsernamePromt(std::function<void(const std::wstring&)> f);

	BEGIN_PROC()
		LRESULT result = 0;
	if (TitleBarLibrary::TitleBarHandleMessage(hWnd, msg, wParam, lParam, titleBar, result))
	{
		return result;
	}
	BEGIN_MAP()
		REGISTER_HANDLER(WM_CREATE, OnCreate)
		REGISTER_HANDLER(WM_DESTROY, OnDestroy)
		REGISTER_HANDLER(WM_PAINT, OnPaint)
		REGISTER_HANDLER(WM_NOTIFY, OnNotify)
		REGISTER_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MAP()
	END_PROC()
};
