#include "UsernamePromt.h"

#include "WindowClass.h"
#include "systemcolors.h"

#include <windowsx.h>
#include <richedit.h>
#include <textserv.h>


UsernamePromt::UsernamePromt(std::function<void(const std::wstring&)> f) :
	function(f)
{

}

LRESULT UsernamePromt::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	titleBar.EnableButtons(TitleBarLibrary::CloseButton | TitleBarLibrary::MinimizeButton);
	titleBar.SetSizingMargins(MARGINS{ 0, 0, 0, 0 });
	TitleBarLibrary::UpdateNCArea(hWnd);

	const auto& colors = titleBar.GetTitleBarColors();

	RECT clientRect{ };
	GetClientRect(hWnd, &clientRect);

	hWndEdit = CreateWindowEx(NULL, MSFTEDIT_CLASS, NULL,
		WS_VISIBLE | WS_CHILD | ES_CENTER,
		clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top - 30,
		hWnd, NULL, NULL, NULL);

	SendMessage(hWndEdit, EM_SETLIMITTEXT, 120, NULL);
	SendMessage(hWndEdit, EM_SETBKGNDCOLOR, 0, SystemColors::IsDarkMode() ? RGB(13, 13, 13) : RGB(200, 200, 200));

	CHARFORMAT cf{ };
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_ALL;

	SendMessage(hWndEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	SendMessage(hWndEdit, EM_SETFONTSIZE, 2, NULL);
	SendMessage(hWndEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	SendMessage(hWndEdit, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf);

	cf = CHARFORMAT{ };
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = colors.defaultTextColor;
	SendMessage(hWndEdit, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf);

	SendMessage(hWndEdit, WM_SETTEXT, NULL, (LPARAM)L"Enter Username");

	HMODULE hmodRichEdit = LoadLibrary(L"Msftedit.dll");
	if (hmodRichEdit == NULL)
	{
		return 0;
	}
	if (IID* IID_ITextServices = (IID*)GetProcAddress(hmodRichEdit, "IID_ITextServices"))
	{
		ComPtr<IUnknown> unknown = nullptr;
		if (SendMessage(hWndEdit, EM_GETOLEINTERFACE, 0, (LPARAM)&unknown))
		{
			ITextServices* textService = nullptr;
			unknown->QueryInterface(*IID_ITextServices, (void**)&textService);
			if (textService)
			{
				textService->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, 0);
			}
		}
	}

	FreeLibrary(hmodRichEdit);

	SendMessage(hWndEdit, EM_SETEVENTMASK, NULL, ENM_KEYEVENTS | ENM_MOUSEEVENTS);

	return 0;
}

LRESULT UsernamePromt::OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!userClicked)
	{
		PostQuitMessage(0);
	}

	return 0;
}

LRESULT UsernamePromt::OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	auto& colors = titleBar.GetTitleBarColors();
	HBRUSH frameBrush = CreateSolidBrush(colors.maximizeButtonColors.defaultForegroundColor);
	HBRUSH backgroundBrush = CreateSolidBrush(SystemColors::IsDarkMode() ? RGB(0, 0, 0) : RGB(255, 255, 255));

	RECT clientRect{ };
	GetClientRect(hWnd, &clientRect);

	HFONT font = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));

	PAINTSTRUCT ps{ };
	HDC hdc = BeginPaint(hWnd, &ps);

	HFONT oldFont = SelectFont(hdc, font);

	RECT drawRect{ };
	SetRect(&drawRect, clientRect.left, clientRect.bottom - 30, clientRect.right, clientRect.bottom);

	int oldBkMode = SetBkMode(hdc, TRANSPARENT);
	COLORREF oldColor = SetTextColor(hdc, colors.defaultTextColor);

	FillRect(hdc, &drawRect, backgroundBrush);
	FrameRect(hdc, &drawRect, frameBrush);
	DrawText(hdc, TEXT("Click to connect"), -1, &drawRect, DT_CENTER | DT_VCENTER);

	SetTextColor(hdc, oldColor);
	SetBkMode(hdc, oldBkMode);

	DeleteFont(SelectFont(hdc, oldFont));
	DeleteBrush(backgroundBrush);
	DeleteBrush(frameBrush);

	EndPaint(hWnd, &ps);

	return 0;
}

LRESULT UsernamePromt::OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	CallCallback();

	return 0;
}

LRESULT UsernamePromt::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);

	switch (nmhdr->code)
	{
		case EN_MSGFILTER:
		{
			if (nmhdr->hwndFrom == hWndEdit)
			{
				MSGFILTER* msgFilter = reinterpret_cast<MSGFILTER*>(lParam);

				if (msgFilter->msg == WM_KEYUP && msgFilter->wParam == VK_RETURN)
				{
					CallCallback();
					return 1;
				}

				if (msgFilter->msg == WM_LBUTTONDOWN && firstTime)
				{
					firstTime = false;
					SendMessage(hWndEdit, WM_SETTEXT, NULL, (LPARAM)TEXT(""));
					SendMessage(hWndEdit, EM_SETEVENTMASK, NULL, ENM_KEYEVENTS);
				}

				return 0;
			}
		}
	}

	return 0;
}

void UsernamePromt::CallCallback()
{
	if (!SendMessage(hWndEdit, WM_GETTEXTLENGTH, NULL, NULL) || firstTime)
	{
		MessageBeep(MB_ICONWARNING);
		return;
	}

	Show(false);
	LRESULT length = SendMessage(hWndEdit, WM_GETTEXTLENGTH, NULL, NULL) + 1;
	wchar_t* username = new wchar_t[length];
	SendMessage(hWndEdit, WM_GETTEXT, length, (LPARAM)username);

	userClicked = true;

	DestroyWindow(hWnd);

	function(username);

	delete[] username;
}
