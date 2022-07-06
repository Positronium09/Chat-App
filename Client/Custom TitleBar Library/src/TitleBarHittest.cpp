#include "pch.h"

#include "TitleBar.h"


LRESULT TitleBarLibrary::TitleBar::OnNCHittest(HWND hWnd, POINT p)
{
	if (RECT iconRect = CalculateIconRect(hWnd, true);
		PtInRect(&iconRect, p))
	{
		return HTSYSMENU;
	}

	if (RECT closeButtonRect = CalculateCloseButtonRect(hWnd, _SCREEN_COORDS);
		PtInRect(&closeButtonRect, p))
	{
		return HTCLOSE;
	}
	else if (RECT maximizeButtonRect = CalculateMaximizeButtonRect(hWnd, _SCREEN_COORDS);
		PtInRect(&maximizeButtonRect, p))
	{
		return HTMAXBUTTON;
	}
	else if (RECT minimizeButtonRect = CalculateMinimizeButtonRect(hWnd, _SCREEN_COORDS);
		PtInRect(&minimizeButtonRect, p))
	{
		return HTMINBUTTON;
	}

	LRESULT hitTestSizingBorders = [&]()
	{
		RECT windowRect{ };
		GetWindowRect(hWnd, &windowRect);

		LRESULT hits[3][3] =
		{
			{ HTTOPLEFT, HTTOP, HTTOPRIGHT },
			{ HTLEFT, HTNOWHERE, HTRIGHT },
			{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT }
		};

		int hitBorderX = 1;
		int hitBorderY = 1;

		if (p.x >= windowRect.left && p.x <= windowRect.left + GetSizingMargins().cxLeftWidth)
		{
			hitBorderX = 0;
		}
		else if (p.x <= windowRect.right && p.x >= windowRect.right - GetSizingMargins().cxRightWidth)
		{
			hitBorderX = 2;
		}
		if (p.y >= windowRect.top && p.y <= windowRect.top + GetSizingMargins().cyTopHeight)
		{
			hitBorderY = 0;
		}
		else if (p.y <= windowRect.bottom && p.y >= windowRect.bottom - GetSizingMargins().cyBottomHeight)
		{
			hitBorderY = 2;
		}

		return hits[hitBorderY][hitBorderX];
	}();

	if (hitTestSizingBorders != HTNOWHERE)
	{
		return hitTestSizingBorders;
	}

	if (RECT titleBarRect = CalculateTitleBarRect(hWnd, _SCREEN_COORDS);
		PtInRect(&titleBarRect, p))
	{
		return HTCAPTION;
	}
	
	return HTCLIENT;
}
