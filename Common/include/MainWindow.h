#pragma once

#include "Window.h"
#include "Titlebar.h"

#include "windowMessages.h"


class MainWindow : public WindowWrapper::WindowBase
{
	private:
	bool firstTime = true;

	HWND CreateRichEdit(COLORREF textColor, COLORREF backgroundColor, DWORD styles = 0);

	protected:
	TitleBarLibrary::TitleBar titleBar{ TitleBarLibrary::TitleBarParams{ } };
	HWND hWndMessages = NULL;
	HWND hWndMessageEnter = NULL;

	static constexpr COLORREF NO_COLORING = RGB(1, 1, 1);

	virtual void Send() = 0;

	void DisplayOnScreen(const wchar_t* toPrint, COLORREF textColor = NO_COLORING);

	LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkGnd(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);

	public:
	virtual ~MainWindow() = default;
};
