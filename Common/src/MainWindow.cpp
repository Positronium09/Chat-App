#include "MainWindow.h"

#include "systemcolors.h"
#include "header.h"

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


HWND MainWindow::CreateRichEdit(COLORREF textColor, COLORREF backgroundColor, DWORD styles)
{
	HWND hWndEdit = CreateWindowEx(NULL, MSFTEDIT_CLASS, NULL,
		WS_VISIBLE | WS_CHILD | styles,
		0, 0, 0, 0,
		hWnd, NULL, NULL, NULL);

	SendMessage(hWndEdit, EM_SETLIMITTEXT, 120, NULL);
	SendMessage(hWndEdit, EM_SETBKGNDCOLOR, 0, backgroundColor);

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
	cf.crTextColor = textColor;
	SendMessage(hWndEdit, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf);

	HMODULE hmodRichEdit = LoadLibrary(L"Msftedit.dll");
	if (hmodRichEdit == NULL)
	{
		return hWndEdit;
	}

	if (IID* IID_ITextServicesFromDll = (IID*)GetProcAddress(hmodRichEdit, "IID_ITextServices"))
	{
		ComPtr<IUnknown> unknown = nullptr;
		if (SendMessage(hWndEdit, EM_GETOLEINTERFACE, 0, (LPARAM)&unknown))
		{
			ITextServices* textService = nullptr;
			unknown->QueryInterface(*IID_ITextServicesFromDll, (void**)&textService);
			if (textService)
			{
				textService->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, 0);
			}
		}
	}

	FreeLibrary(hmodRichEdit);

	return hWndEdit;
}

LRESULT MainWindow::OnCreate(HWND p_hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(p_hWnd);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	titleBar.EnableButtons(TitleBarLibrary::CloseButton | TitleBarLibrary::MinimizeButton);
	titleBar.SetSizingMargins(MARGINS{ 0, 0, 0, 0 });
	TitleBarLibrary::UpdateNCArea(hWnd);

	const auto& colors = titleBar.GetTitleBarColors();

	RECT clientRect{ };
	GetClientRect(hWnd, &clientRect);

	COLORREF backgroundColor = SystemColors::IsDarkMode() ? RGB(13, 13, 13) : RGB(200, 200, 200);

	hWndMessages = CreateRichEdit(colors.defaultTextColor, backgroundColor, ES_MULTILINE | ES_READONLY);
	hWndMessageEnter = CreateRichEdit(colors.defaultTextColor, backgroundColor);

	SendMessage(hWndMessageEnter, EM_SETEVENTMASK, NULL, ENM_KEYEVENTS | ENM_MOUSEEVENTS);

	if (hWndMessages == NULL || hWndMessageEnter == NULL)
	{
		MessageBox(NULL, TEXT("Cannot create rich edit controls"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return -1;
	}

	SetWindowPos(hWndMessages, NULL, 1, 1, clientRect.right - clientRect.left - 2, clientRect.bottom - clientRect.top - 51, SWP_NOOWNERZORDER);
	SetWindowPos(hWndMessageEnter, NULL, 1, clientRect.bottom - 49, clientRect.right - clientRect.left - 2, 48, SWP_NOOWNERZORDER);

	SendMessage(hWndMessageEnter, WM_SETTEXT, NULL, (LPARAM)TEXT("Enter a message"));

	PARAFORMAT2 paraFormat{ };
	paraFormat.cbSize = sizeof(PARAFORMAT2);
	paraFormat.dwMask = PFM_LINESPACING;
	SendMessage(hWndMessages, EM_GETPARAFORMAT, NULL, (LPARAM)&paraFormat);

	paraFormat.dyLineSpacing = 10;

	SendMessage(hWndMessages, EM_SETPARAFORMAT, NULL, (LPARAM)&paraFormat);

	return 0;
}

void MainWindow::DisplayOnScreen(const wchar_t* toPrint, COLORREF textColor)
{
	CHARRANGE selection{ };
	SendMessage(hWndMessages, EM_EXGETSEL, NULL, (LPARAM)&selection);

	GETTEXTLENGTHEX textLength{ };
	textLength.flags = GTL_NUMCHARS;
	LRESULT length = SendMessage(hWndMessages, EM_GETTEXTLENGTHEX, (WPARAM)&textLength, NULL);

	SendMessage(hWndMessages, EM_SETSEL, length, length);

	SendMessage(hWndMessages, EM_REPLACESEL, NULL, (LPARAM)toPrint);

	if (textColor != RGB(1, 1, 1))
	{
		if (length != 0)
		{
			length--;
		}

		LRESULT newLength = SendMessage(hWndMessages, EM_GETTEXTLENGTHEX, (WPARAM)&textLength, NULL);

		CHARRANGE newTextRange{
			static_cast<LONG>(length),
			std::clamp(static_cast<LONG>(length + std::wcslen(toPrint)) + 2, 0l, static_cast<LONG>(newLength)) };

		SendMessage(hWndMessages, EM_EXSETSEL, NULL, (LPARAM)&newTextRange);

		CHARFORMAT cf{ };
		cf.cbSize = sizeof(CHARFORMAT);
		cf.dwMask = CFM_COLOR;
		cf.crTextColor = textColor;
		SendMessage(hWndMessages, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}

	SendMessage(hWndMessages, EM_EXSETSEL, NULL, (LPARAM)&selection);
}

LRESULT MainWindow::OnNotify(HWND p_hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(p_hWnd);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);

	switch (nmhdr->code)
	{
		case EN_MSGFILTER:
		{
			if (nmhdr->hwndFrom == hWndMessageEnter)
			{
				MSGFILTER* msgFilter = reinterpret_cast<MSGFILTER*>(lParam);

				if (msgFilter->msg == WM_KEYUP && msgFilter->wParam == VK_RETURN)
				{
					Send();
					return 1;
				}

				if (msgFilter->msg == WM_LBUTTONDOWN && firstTime)
				{
					firstTime = false;
					SendMessage(hWndMessageEnter, WM_SETTEXT, NULL, (LPARAM)TEXT(""));
					SendMessage(hWndMessageEnter, EM_SETEVENTMASK, NULL, ENM_KEYEVENTS);
				}

				return 0;
			}
		}
	}

	return 0;
}

LRESULT MainWindow::OnEraseBkGnd(HWND p_hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(p_hWnd);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	auto& colors = titleBar.GetTitleBarColors();
	HBRUSH brush = CreateSolidBrush(colors.maximizeButtonColors.defaultForegroundColor);

	HDC hdc = reinterpret_cast<HDC>(wParam);

	RECT clientRect{ };
	GetClientRect(hWnd, &clientRect);

	FillRect(hdc, &clientRect, brush);

	DeleteBrush(brush);
	return 1;
}
