#include "pch.h"

#include "TitleBar.h"
#include "Direct2DResources.h"
#include "WICResources.h"
#include "utils.h"

#include <utility>
#include <algorithm>


#define DCX_USESTYLE 0x00010000


HRGN CopyRegion(HRGN hrgn)
{
	HRGN returnRgn = hrgn;

	if (returnRgn)
	{
		RECT rect = { 0 };

		returnRgn = CreateRectRgnIndirect(&rect);

		if (!CombineRgn(returnRgn, hrgn, NULL, RGN_COPY))
		{
			DeleteRgn(returnRgn);
		}
	}

	return returnRgn;
}

void InflateRect(D2D1_RECT_F& rect, FLOAT dx, FLOAT dy)
{
	rect.left += -dx;
	rect.top += -dy;
	rect.right += dx;
	rect.bottom += dy;
}

D2D1_RECT_F CalculateSmallRect(const RECT rect, const float delta = 6.0f)
{
	using namespace TitleBarLibrary;

	auto smallRect = ConvertRectToRectF(rect);

	const auto middle = D2D1::Point2F((smallRect.left + smallRect.right) / 2, (smallRect.top + smallRect.bottom) / 2);
	
	#pragma push_macro("min")
	#undef min

	const auto offset = std::min(RectWidth(rect), RectHeight(rect)) / delta;

	#pragma pop_macro("min")

	smallRect = D2D1::RectF(
		middle.x - offset, middle.y - offset,
		middle.x + offset, middle.y + offset);

	return smallRect;
}

std::pair<COLORREF, COLORREF> GetButtonColor(int buttonState, const TitleBarLibrary::ButtonColors& buttonColors, bool hasFocus)
{
	using namespace TitleBarLibrary;

	if (hasFocus)
	{
		switch (buttonState)
		{
			case _BS_Default:
			{
				return { buttonColors.defaultForegroundColor, buttonColors.defaultBackgroundColor };
			}
			case _BS_Hover:
			{
				return { buttonColors.hoverForegroundColor, buttonColors.hoverBackgroundColor };
			}
			case _BS_Pressed:
			{
				return { buttonColors.pressedForegroundColor, buttonColors.pressedBackgroundColor };
			}
			case _BS_Disabled:
			{
				return { buttonColors.disabledForegroundColor, buttonColors.disabledBackgroundColor };
			}
		}
	}
	else
	{
		switch (buttonState)
		{
			case _BS_Pressed:
			case _BS_Default:
			{
				return { buttonColors.noFocusForegroundColor, buttonColors.noFocusBackgroundColor };
			}
			case _BS_Hover:
			{
				return { buttonColors.noFocusHoverForegroundColor, buttonColors.noFocusHoverBackgroundColor };
			}
			case _BS_Disabled:
			{
				return { buttonColors.noFocusDisabledForegroundColor, buttonColors.noFocusDisabledBackgroundColor };
			}
		}
	}
	return { };
}

void TitleBarLibrary::TitleBar::DrawCloseButton(HWND hWnd)
{
	RECT closeButtonRect = CalculateCloseButtonRect(hWnd, _CLIENT_COORDS);
	if (IsRectEmpty(&closeButtonRect))
	{
		return;
	}

	auto& rt = direct2dResource.GetRenderTarget();

	auto [foregroundColor, backgroundColor] = GetButtonColor(closeButtonState, colors.closeButtonColors, hasFocus);

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	rt->CreateSolidColorBrush(ConvertColorrefToColorF(backgroundColor), &brush);

	auto smallRect = CalculateSmallRect(closeButtonRect);

	auto antialiasMode = rt->GetAntialiasMode();
	rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	rt->FillRectangle(ConvertRectToRectF(closeButtonRect), brush);
	rt->DrawRectangle(ConvertRectToRectF(closeButtonRect), brush);

	brush->SetColor(ConvertColorrefToColorF(foregroundColor));
	rt->DrawLine(D2D1::Point2F(smallRect.left, smallRect.top), D2D1::Point2F(smallRect.right, smallRect.bottom),
		brush, 1.2f);
	rt->DrawLine(D2D1::Point2F(smallRect.right, smallRect.top), D2D1::Point2F(smallRect.left, smallRect.bottom),
		brush, 1.2f);

	rt->SetAntialiasMode(antialiasMode);
}

void TitleBarLibrary::TitleBar::DrawMaximizeButton(HWND hWnd)
{
	RECT maximizeButtonRect = CalculateMaximizeButtonRect(hWnd, _CLIENT_COORDS);
	if (IsRectEmpty(&maximizeButtonRect))
	{
		return;
	}

	auto& rt = direct2dResource.GetRenderTarget();

	auto [foregroundColor, backgroundColor] = GetButtonColor(maximizeButtonState, colors.maximizeButtonColors, hasFocus);

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	rt->CreateSolidColorBrush(ConvertColorrefToColorF(backgroundColor), &brush);

	auto smallRect = CalculateSmallRect(maximizeButtonRect);

	auto antialiasMode = rt->GetAntialiasMode();
	rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	rt->FillRectangle(ConvertRectToRectF(maximizeButtonRect), brush);
	rt->DrawRectangle(ConvertRectToRectF(maximizeButtonRect), brush);

	brush->SetColor(ConvertColorrefToColorF(foregroundColor));
	
	if (IsMaximized(hWnd))
	{
		smallRect.left -= 1;
		smallRect.right -= 1;
		auto backRect = smallRect;
		
		backRect.left += 2;
		backRect.top -= 2;
		backRect.right += 2;
		backRect.bottom -= 2;

		InflateRect(backRect, -1, -1);
		InflateRect(smallRect, -1, -1);

		rt->DrawRectangle(backRect, brush);
	}

	brush->SetColor(ConvertColorrefToColorF(backgroundColor));
	rt->FillRectangle(smallRect, brush);
	brush->SetColor(ConvertColorrefToColorF(foregroundColor));
	rt->DrawRectangle(smallRect, brush);

	rt->SetAntialiasMode(antialiasMode);
}

void TitleBarLibrary::TitleBar::DrawMinimizeButton(HWND hWnd)
{
	RECT minimizeButtonRect = CalculateMinimizeButtonRect(hWnd, _CLIENT_COORDS);
	if (IsRectEmpty(&minimizeButtonRect))
	{
		return;
	}

	auto& rt = direct2dResource.GetRenderTarget();

	auto [foregroundColor, backgroundColor] = GetButtonColor(minimizeButtonState, colors.minimizeButtonColors, hasFocus);

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	rt->CreateSolidColorBrush(ConvertColorrefToColorF(backgroundColor), &brush);

	auto smallRect = CalculateSmallRect(minimizeButtonRect);
	InflateRect(smallRect, 1.f, 0);

	auto antialiasMode = rt->GetAntialiasMode();
	rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	rt->FillRectangle(ConvertRectToRectF(minimizeButtonRect), brush);
	rt->DrawRectangle(ConvertRectToRectF(minimizeButtonRect), brush);

	const auto middle = (smallRect.top + smallRect.bottom) / 2.0f + 1.f;

	brush->SetColor(ConvertColorrefToColorF(foregroundColor));
	rt->DrawLine(D2D1::Point2F(smallRect.left, middle), D2D1::Point2F(smallRect.right, middle),
		brush, 1.f);

	rt->SetAntialiasMode(antialiasMode);
}

void TitleBarLibrary::TitleBar::DrawIcon(HWND hWnd)
{
	auto& rt = direct2dResource.GetRenderTarget();

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	rt->CreateSolidColorBrush(D2D1::ColorF(0x7d7d7d), &brush);

	RECT iconRect = CalculateIconRect(hWnd, _CLIENT_COORDS);

	HICON icon = NULL;
	icon = reinterpret_cast<HICON>(SendMessage(hWnd, WM_GETICON, ICON_SMALL, GetDpiForWindow(hWnd)));

	if (icon == NULL)
	{
		icon = reinterpret_cast<HICON>(GetClassLongPtr(hWnd, GCLP_HICONSM));
	}

	if (icon == NULL)
	{
		icon = reinterpret_cast<HICON>(LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
	}

	ComPtr<ID2D1Bitmap> bmp = nullptr;
	HRESULT hr = WICResources::GetDirect2DBitmapFromHICON(rt, icon, &bmp);

	if (FAILED(hr))
	{
		return;
	}

	D2D1_SIZE_F size = bmp->GetSize();

	rt->DrawBitmap(bmp, ConvertRectToRectF(iconRect), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1::RectF(0, 0, size.width, size.height));
}

void TitleBarLibrary::TitleBar::DrawWindowText(HWND hWnd)
{
	auto& rt = direct2dResource.GetRenderTarget();

	SIZE_T textLength = SendMessage(hWnd, WM_GETTEXTLENGTH, NULL, NULL) + 1; // +1 for NULL character
	WCHAR* windowText = new WCHAR[textLength];

	SendMessage(hWnd, WM_GETTEXT, textLength, reinterpret_cast<LPARAM>(windowText));

	RECT textRect = CalculateTextRect(hWnd, _CLIENT_COORDS);

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	rt->CreateSolidColorBrush(ConvertColorrefToColorF(hasFocus ? colors.defaultTextColor : colors.noFocusTextColor), &brush);

	rt->DrawTextW(windowText, static_cast<UINT32>(textLength), directWriteResource.GetTextFormat(), ConvertRectToRectF(textRect), brush);
}

LRESULT TitleBarLibrary::TitleBar::OnNCPaint(HWND hWnd, HRGN hrgn)
{
	HRGN copiedRgn = CopyRegion(hrgn);

	HDC hdc = GetDCEx(hWnd, copiedRgn, DCX_WINDOW | DCX_INTERSECTRGN | DCX_LOCKWINDOWUPDATE | DCX_USESTYLE | DCX_CACHE);
	if (hdc == NULL)
	{
		DeleteRgn(copiedRgn);
		return 0;
	}

	auto& rt = direct2dResource.GetRenderTarget();


	RECT windowRect = { 0 };
	GetWindowRect(hWnd, &windowRect);

	RECT windowDrawRect = { 0, 0, RectWidth(windowRect), RectHeight(windowRect) };

	rt->BindDC(hdc, &windowDrawRect);

	rt->BeginDraw();

	rt->Clear(ConvertColorrefToColorF(hasFocus ? colors.defaultFrameColor : colors.noFocusFrameColor));


	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	rt->CreateSolidColorBrush(ConvertColorrefToColorF(hasFocus ? colors.defaultBackgroundColor: colors.noFocusBackgroundColor), &brush);

	rt->FillRectangle(ConvertRectToRectF(CalculateTitleBarRect(hWnd, _CLIENT_COORDS)), brush);

	DrawWindowText(hWnd);

	DrawCloseButton(hWnd);
	DrawMaximizeButton(hWnd);
	DrawMinimizeButton(hWnd);

	if (showIcon)
	{
		DrawIcon(hWnd);
	}

	if (rt->EndDraw() == D2DERR_RECREATE_TARGET)
	{
		direct2dResource.InitializeRenderTarget();
	}

	ReleaseDC(hWnd, hdc);

	return 0;
}
