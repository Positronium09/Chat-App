#include "pch.h"
#include "Titlebar.h"

#include "systemcolors.h"
#include "utils.h"
#include "Direct2DResources.h"
#include "WICResources.h"
#include "DirectWriteResources.h"

#include <algorithm>

#pragma comment (lib, "uxtheme.lib")


//? http://www.devsuperpage.com/search/Articles.asp?ArtID=1071718
// Found those while looking for what message#174 is

#define WM_NCUAHDRAWCAPTION 0x00AE
#define WM_NCUAHDRAWFRAME 0x00AF


BOOL TitleBarLibrary::UpdateNCArea(HWND hWnd)
{
	return SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
}

void TrackMouse(HWND hWnd)
{
	TRACKMOUSEEVENT tme{ };
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
	tme.dwHoverTime = 0;
	tme.hwndTrack = hWnd;

	TrackMouseEvent(&tme);
}

HRESULT TitleBarLibrary::InitializeTitleBarLibrary()
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	hr = Direct2DResources::InitializeResources();
	hr = WICResources::InitializeResources();
	hr = DirectWriteResources::InitializeResources();

	return hr;
}

bool TitleBarLibrary::TitleBarHandleMessage(HWND hWnd, UINT msg, WPARAM& wParam, LPARAM& lParam, TitleBar& titleBar, LRESULT& lResult)
{
	lResult = 0;

	switch (msg)
	{
		case WM_NCCREATE:
		{
			// Fix system menu
			GetSystemMenu(hWnd, FALSE);

			SetWindowTheme(hWnd, L" ", L" ");

			TrackMouse(hWnd);

			titleBar._hWnd = hWnd;

			return UNHANDLED;
		}

		case WM_NCCALCSIZE:
		{
			if (!wParam)
			{
				return UNHANDLED;
			}

			const auto& frameMargins = titleBar.GetFrameMargins();

			NCCALCSIZE_PARAMS* sz = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

			if (IsMaximized(hWnd))
			{
				sz->rgrc[0].top = titleBar.GetHeight();
				sz->rgrc[0].left += frameMargins.cxLeftWidth;
				sz->rgrc[0].right -= frameMargins.cxRightWidth;
				sz->rgrc[0].bottom -= frameMargins.cyBottomHeight;
			}
			else
			{
				sz->rgrc[0].top += frameMargins.cyTopHeight + titleBar.GetHeight();
				sz->rgrc[0].left += frameMargins.cxLeftWidth;
				sz->rgrc[0].right -= frameMargins.cxRightWidth;
				sz->rgrc[0].bottom -= frameMargins.cyBottomHeight;
			}

			return HANDLED;
		}

		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO minMaxInfo = reinterpret_cast<LPMINMAXINFO>(lParam);

			NCCALCSIZE_PARAMS params = {  };
			SendMessage(hWnd, WM_NCCALCSIZE, TRUE, reinterpret_cast<LPARAM>(&params));

			// Add 1 so GetDCEx wont fail
			// Idk why it does this
			minMaxInfo->ptMinTrackSize.y = titleBar.GetHeight() + titleBar.GetFrameMargins().cyTopHeight + titleBar.GetFrameMargins().cyBottomHeight + 1;

			const LONG xSize = titleBar.GetCloseButtonSize().cx + titleBar.GetMaximizeButtonSize().cx + titleBar.GetMaximizeButtonSize().cx;

			minMaxInfo->ptMinTrackSize.x = xSize + titleBar.GetFrameMargins().cxLeftWidth + titleBar.GetFrameMargins().cxRightWidth + 1;

			return UNHANDLED;
		}

		case WM_SIZE:
		{
			UpdateNCArea(hWnd);

			return UNHANDLED;
		}

		case WM_NCMOUSEMOVE:
		{
			TrackMouse(hWnd);

			if (titleBar.UpdateButtonState(wParam, _BS_Hover))
			{
				UpdateNCArea(hWnd);
			}

			return UNHANDLED;
		}

		case WM_NCMOUSELEAVE:
		{
			if (titleBar.UpdateButtonState(HTNOWHERE, 0))
			{
				UpdateNCArea(hWnd);
			}

			return UNHANDLED;
		}

		case WM_NCACTIVATE:
		{
			if (IsMinimized(hWnd))
			{
				return UNHANDLED;
			}

			lResult = wParam;
			titleBar.hasFocus = wParam;

			UpdateNCArea(hWnd);

			return HANDLED;
		}

		case WM_NCHITTEST:
		{
			lResult = titleBar.OnNCHittest(hWnd, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });

			return HANDLED;
		}

		case WM_NCLBUTTONDOWN:
		{
			if (titleBar.UpdateButtonState(wParam, _BS_Pressed))
			{
				UpdateNCArea(hWnd);
			}

			switch (wParam)
			{
				case HTSYSMENU:
				{
					RECT titleBarRect = titleBar.CalculateTitleBarRect(hWnd, true);

					(void)DefWindowProc(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
					(void)DefWindowProc(hWnd, WM_NCLBUTTONUP, HTCAPTION, lParam);

					HMENU sysMenu = GetSystemMenu(hWnd, FALSE);

					auto command = TrackPopupMenu(sysMenu, TPM_RETURNCMD, titleBarRect.left, titleBarRect.bottom, NULL, hWnd, nullptr);

					SendMessage(hWnd, WM_SYSCOMMAND, command, lParam);

					return HANDLED;
				}

				case HTCLOSE:
				case HTMAXBUTTON:
				case HTMINBUTTON:
				{
					return HANDLED;
				}
			}
			return UNHANDLED;
		}

		case WM_NCLBUTTONUP:
		{
			switch (wParam)
			{
				case HTCLOSE:
				{
					if (titleBar.closeButtonState == _BS_Pressed)
					{
						DestroyWindow(hWnd);
					}
					return HANDLED;
				}
				case HTMAXBUTTON:
				{
					if (titleBar.maximizeButtonState != _BS_Pressed)
					{
						return UNHANDLED;
					}
					if (IsMaximized(hWnd))
					{
						ShowWindow(hWnd, SW_RESTORE);
					}
					else
					{
						ShowWindow(hWnd, SW_MAXIMIZE);
					}					
					return HANDLED;
				}
				case HTMINBUTTON:
				{
					if (titleBar.minimizeButtonState == _BS_Pressed)
					{
						ShowWindow(hWnd, SW_MINIMIZE);
					}
					return HANDLED;
				}
			}

			if (titleBar.UpdateButtonState(HTNOWHERE, 0))
			{
				UpdateNCArea(hWnd);
			}

			return UNHANDLED;
		}

		case WM_NCUAHDRAWCAPTION:
		case WM_NCUAHDRAWFRAME:
		case WM_NCPAINT:
		{
			lResult = titleBar.OnNCPaint(hWnd, reinterpret_cast<HRGN>(wParam));

			RedrawWindow(hWnd, NULL, NULL, RDW_INTERNALPAINT | RDW_NOFRAME | RDW_VALIDATE);
			UpdateWindow(hWnd);

			return HANDLED;
		}
	}

	return UNHANDLED;
}

TitleBarLibrary::TitleBarColors TitleBarLibrary::GetDefaultTitleBarColors()
{
	const auto BLACK = RGB(0x00, 0x00, 0x00);
	const auto ELTGRAY = RGB(0xc6, 0xc6, 0xc6);
	const auto LTGRAY = RGB(0xa6, 0xa6, 0xa6);
	const auto GRAY = RGB(0x7f, 0x7f, 0x7f);
	const auto DKGRAY = RGB(0x3f, 0x3f, 0x3f);
	const auto EDKGRAY = RGB(0x1b, 0x1b, 0x1b);

	TitleBarColors colors{ };

	auto& closeButtonColors = colors.closeButtonColors;
	auto& maximizeButtonColors = colors.maximizeButtonColors;
	auto& minimizeButtonColors = colors.minimizeButtonColors;

	const bool darkMode = SystemColors::IsDarkMode();

	colors.defaultTextColor = SystemColors::GetTextColor();
	colors.defaultBackgroundColor = SystemColors::GetBackgroundColor();
	colors.defaultFrameColor = colors.defaultBackgroundColor;

	colors.noFocusTextColor = GRAY;
	colors.noFocusBackgroundColor = darkMode ? EDKGRAY : ELTGRAY;
	colors.noFocusFrameColor = colors.noFocusBackgroundColor;

	COLORREF foregroundColor = SystemColors::GetAccentColor();
	COLORREF noFocusForegroundColor = darkMode ? SystemColors::GetAccentColorDark3() : SystemColors::GetAccentColorLight3();
	COLORREF noFocusHoverForegroundColor = darkMode ? SystemColors::GetAccentColorDark2() : SystemColors::GetAccentColorLight2();
	COLORREF noFocusHoverBackgroundColor = darkMode ? DKGRAY : LTGRAY;

	COLORREF pressedForegroundColor = colors.defaultBackgroundColor;
	COLORREF pressedBackgroundColor = darkMode ? SystemColors::GetAccentColorDark1() : SystemColors::GetAccentColorLight1();

	closeButtonColors = 
	{
		foregroundColor, colors.defaultBackgroundColor, // Normal
		BLACK, RGB(255, 0, 0), // Hover
		GRAY, colors.defaultBackgroundColor, // Disabled
		BLACK, RGB(225, 0, 0), // Pressed
		RGB(200, 0, 0), colors.noFocusBackgroundColor, // NoFocus
		RGB(215, 0, 0), noFocusHoverBackgroundColor, // NoFocusHover
		LTGRAY, colors.noFocusBackgroundColor // NoFocusDisabled
	};
	maximizeButtonColors = 
	{
		foregroundColor, colors.defaultBackgroundColor, // Normal
		foregroundColor, darkMode ? DKGRAY : LTGRAY, // Hover
		GRAY, colors.defaultBackgroundColor, // Disabled
		pressedForegroundColor, pressedBackgroundColor, // Pressed
		noFocusForegroundColor, colors.noFocusBackgroundColor, // NoFocus
		noFocusHoverForegroundColor, noFocusHoverBackgroundColor, // NoFocusHover
		LTGRAY, colors.noFocusBackgroundColor // NoFocusDisabled
	};
	minimizeButtonColors =
	{
		foregroundColor, colors.defaultBackgroundColor, // Normal
		foregroundColor, darkMode ? DKGRAY : LTGRAY, // Hover
		GRAY, colors.defaultBackgroundColor, // Disabled
		pressedForegroundColor, pressedBackgroundColor, // Pressed
		noFocusForegroundColor, colors.noFocusBackgroundColor, // NoFocus
		noFocusHoverForegroundColor, noFocusHoverBackgroundColor, // NoFocusHover
		LTGRAY, colors.noFocusBackgroundColor // NoFocusDisabled
	};

	return colors;
}



TitleBarLibrary::TitleBar::TitleBar(const TitleBarParams& titleBarParams) : 
	metrics(titleBarParams.metrics),
	showIcon(titleBarParams.showIcon),
	colors(GetDefaultTitleBarColors())
{
	EnableButtons(titleBarParams.enabledButtons);
	directWriteResource.CreateTextFormat(titleBarParams.fontFamilyName);
}

TitleBarLibrary::TitleBar::TitleBar(const TitleBarColors& p_colors, const TitleBarParams& titleBarParams) : 
	metrics(titleBarParams.metrics),
	showIcon(titleBarParams.showIcon),
	colors(p_colors)
{
	EnableButtons(titleBarParams.enabledButtons);
	directWriteResource.CreateTextFormat(titleBarParams.fontFamilyName);
}

TitleBarLibrary::TitleBar::~TitleBar()
{

}

void TitleBarLibrary::TitleBar::EnableButtons(int enabledButtons)
{
	closeButtonState = (enabledButtons & CloseButton ? 0 : _BS_Disabled);
	maximizeButtonState = (enabledButtons & MaximizeButton ? 0 : _BS_Disabled);
	minimizeButtonState = (enabledButtons & MinimizeButton ? 0 : _BS_Disabled);
}

void TitleBarLibrary::TitleBar::SetShowIcon(bool p_showIcon)
{
	showIcon = p_showIcon;

	UpdateNCArea(_hWnd);
}

void TitleBarLibrary::TitleBar::SetTitleBarColors(const TitleBarColors& titleBarColors)
{
	colors = titleBarColors;

	UpdateNCArea(_hWnd);
}

void TitleBarLibrary::TitleBar::SetHeight(LONG p_height)
{
	metrics.height = p_height;

	UpdateNCArea(_hWnd);
}

void TitleBarLibrary::TitleBar::SetFrameMargins(MARGINS margins)
{
	metrics.frameMargins = margins;

	UpdateNCArea(_hWnd);
}

void TitleBarLibrary::TitleBar::SetSizingMargins(MARGINS margins)
{
	metrics.sizingMargins = margins;
}

void TitleBarLibrary::TitleBar::SetCloseButtonSize(SIZE size)
{
	metrics.closeButtonSize = size;

	UpdateNCArea(_hWnd);
}

void TitleBarLibrary::TitleBar::SetMaximizeButtonSize(SIZE size)
{
	metrics.maximizeButtonSize = size;

	UpdateNCArea(_hWnd);
}

void TitleBarLibrary::TitleBar::SetMinimizeButtonSize(SIZE size)
{
	metrics.minimizeButtonSize = size;

	UpdateNCArea(_hWnd);
}

void TitleBarLibrary::TitleBar::SetTitleBarMetrics(const TitleBarMetrics& titleBarMetrics)
{
	metrics = titleBarMetrics;
}

bool TitleBarLibrary::TitleBar::GetShowIcon() const
{
	return showIcon;
}

TitleBarLibrary::TitleBarColors& TitleBarLibrary::TitleBar::GetTitleBarColors()
{
	return colors;
}

const TitleBarLibrary::TitleBarColors& TitleBarLibrary::TitleBar::GetTitleBarColors() const
{
	return colors;
}

TitleBarLibrary::TitleBarMetrics& TitleBarLibrary::TitleBar::GetTitleBarMetrics()
{
	return metrics;
}

const TitleBarLibrary::TitleBarMetrics& TitleBarLibrary::TitleBar::GetTitleBarMetrics() const
{
	return metrics;
}

LONG TitleBarLibrary::TitleBar::GetHeight() const
{
	return metrics.height;
}

MARGINS TitleBarLibrary::TitleBar::GetFrameMargins() const
{
	return metrics.frameMargins;
}

MARGINS TitleBarLibrary::TitleBar::GetSizingMargins() const
{
	return metrics.sizingMargins;
}

SIZE TitleBarLibrary::TitleBar::GetCloseButtonSize() const
{
	return metrics.closeButtonSize;
}

SIZE TitleBarLibrary::TitleBar::GetMaximizeButtonSize() const
{
	return metrics.maximizeButtonSize;
}

SIZE TitleBarLibrary::TitleBar::GetMinimizeButtonSize() const
{
	return metrics.minimizeButtonSize;
}

RECT TitleBarLibrary::TitleBar::CalculateTitleBarRect(HWND hWnd, bool screenCoords)
{
	RECT titleBarRect{ };
	
	RECT windowRect{ };
	GetWindowRect(hWnd, &windowRect);

	if (screenCoords == _SCREEN_COORDS)
	{
		titleBarRect = windowRect;
		titleBarRect.bottom = titleBarRect.top + GetHeight();
	}
	else
	{
		titleBarRect = { 0, 0, RectWidth(windowRect), RectHeight(windowRect) };
	}

	if (IsMaximized(hWnd))
	{
		if (screenCoords)
		{
			titleBarRect.top = 0;
			titleBarRect.left = 0;
		}
		else
		{
			titleBarRect.top = 8;
			titleBarRect.left = 8;
		}
		titleBarRect.right -= 8;
	}
	else
	{
		titleBarRect.top += GetFrameMargins().cyTopHeight;
		titleBarRect.left += GetFrameMargins().cxLeftWidth;
		titleBarRect.right -= GetFrameMargins().cxRightWidth;
	}

	titleBarRect.bottom = titleBarRect.top + GetHeight();

	return titleBarRect;
}

// Helper For Calculate...ButtonRect
LONG GetButtonHeight(LONG buttonHeight, LONG titleBarHeight)
{
	#pragma push_macro("min")
	#undef min

	return std::min(buttonHeight == -1 ? titleBarHeight : buttonHeight, titleBarHeight);

	#pragma pop_macro("min")
}

RECT TitleBarLibrary::TitleBar::CalculateCloseButtonRect(HWND hWnd, bool screenCoords)
{
	RECT closeButtonRect{ };

	RECT titleBarRect = CalculateTitleBarRect(hWnd, screenCoords);
	
	if (!IsMaximized(hWnd))
	{
		titleBarRect.top += 1;
	}

	closeButtonRect.left = titleBarRect.right - GetCloseButtonSize().cx;
	closeButtonRect.top = titleBarRect.top;
	closeButtonRect.right = titleBarRect.right;
	closeButtonRect.bottom = titleBarRect.top + GetButtonHeight(GetCloseButtonSize().cy, GetHeight());

	return closeButtonRect;
}

RECT TitleBarLibrary::TitleBar::CalculateMaximizeButtonRect(HWND hWnd, bool screenCoords)
{
	RECT maximizeButtonRect{ };

	RECT titleBarRect = CalculateTitleBarRect(hWnd, screenCoords);
	
	if (!IsMaximized(hWnd))
	{
		titleBarRect.top += 1;
	}

	maximizeButtonRect.left = titleBarRect.right - (GetMaximizeButtonSize().cx + GetCloseButtonSize().cx);
	maximizeButtonRect.top = titleBarRect.top;
	maximizeButtonRect.right = titleBarRect.right - GetCloseButtonSize().cx;
	maximizeButtonRect.bottom = titleBarRect.top + GetButtonHeight(GetMaximizeButtonSize().cy, GetHeight());

	return maximizeButtonRect;
}

RECT TitleBarLibrary::TitleBar::CalculateMinimizeButtonRect(HWND hWnd, bool screenCoords)
{
	RECT minimizeButtonRect{ };

	RECT titleBarRect = CalculateTitleBarRect(hWnd, screenCoords);

	if (!IsMaximized(hWnd))
	{
		titleBarRect.top += 1;
	}

	minimizeButtonRect.left = titleBarRect.right - (GetMinimizeButtonSize().cx + GetMaximizeButtonSize().cx + GetCloseButtonSize().cx);
	minimizeButtonRect.top = titleBarRect.top;
	minimizeButtonRect.right = titleBarRect.right - (GetMaximizeButtonSize().cx + GetCloseButtonSize().cx);
	minimizeButtonRect.bottom = titleBarRect.top + GetButtonHeight(GetMinimizeButtonSize().cy, GetHeight());

	return minimizeButtonRect;
}

RECT TitleBarLibrary::TitleBar::CalculateIconRect(HWND hWnd, bool screenCoords)
{
	RECT iconRect = CalculateTitleBarRect(hWnd, screenCoords);

	if (!showIcon)
	{
		iconRect.right = iconRect.left;
		iconRect.bottom = iconRect.top;
		return iconRect;
	}

	const int iconWidth = GetSystemMetrics(SM_CXSMICON);
	const int iconHeight = GetSystemMetrics(SM_CYSMICON);

	iconRect.right = iconRect.left + GetHeight();
	iconRect.bottom = iconRect.top + GetHeight();

	const LONG xOffset = (GetHeight() - iconWidth) / 2;
	const LONG yOffset = (GetHeight() - iconHeight) / 2;

	iconRect.left += xOffset;
	iconRect.top += yOffset;
	iconRect.right -= xOffset;
	iconRect.bottom -= yOffset;

	return iconRect;
}

RECT TitleBarLibrary::TitleBar::CalculateTextRect(HWND hWnd, bool screenCoords)
{
	RECT textRect = CalculateTitleBarRect(hWnd, screenCoords);
	RECT iconRect = CalculateIconRect(hWnd, screenCoords);
	RECT minimizeButtonRect = CalculateMinimizeButtonRect(hWnd, screenCoords);

	#pragma push_macro("max")
	#undef max

	textRect.left += std::max(GetHeight(), 10l);
	textRect.right = minimizeButtonRect.left;

	#pragma pop_macro("max")

	OffsetRect(&textRect, 0, -1);

	return textRect;
}

bool TitleBarLibrary::TitleBar::UpdateButtonState(WPARAM wParam, int state)
{
	int oldCloseButtonState = closeButtonState;
	int oldMaximizeButtonState = maximizeButtonState;
	int oldMinimizeButtonState = minimizeButtonState;

	switch (wParam)
	{
		case HTCLOSE:
		{
			if (closeButtonState != _BS_Disabled)
			{
				closeButtonState = closeButtonState == _BS_Pressed ? _BS_Pressed : state;
			}
			maximizeButtonState = maximizeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			minimizeButtonState = minimizeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			break;
		}
		case HTMAXBUTTON:
		{
			closeButtonState = closeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			if (maximizeButtonState != _BS_Disabled)
			{
				maximizeButtonState = maximizeButtonState == _BS_Pressed ? _BS_Pressed : state;
			}
			minimizeButtonState = minimizeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			break;
		}
		case HTMINBUTTON:
		{
			closeButtonState = closeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			maximizeButtonState = maximizeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			if (minimizeButtonState != _BS_Disabled)
			{
				minimizeButtonState = minimizeButtonState == _BS_Pressed ? _BS_Pressed : state;
			}
			break;
		}
		default:
		{
			closeButtonState = closeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			maximizeButtonState = maximizeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			minimizeButtonState = minimizeButtonState == _BS_Disabled ? _BS_Disabled : _BS_Default;
			break;
		}
	}

	return oldCloseButtonState != closeButtonState || 
			oldMaximizeButtonState != maximizeButtonState || 
			oldMinimizeButtonState != minimizeButtonState;
}
