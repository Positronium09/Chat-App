#pragma once

#include "pch.h"

#include <d2d1_1.h>


namespace TitleBarLibrary
{
	constexpr LONG RectWidth(const RECT& rc) { return rc.right - rc.left; }
	constexpr LONG RectHeight(const RECT& rc) { return rc.bottom - rc.top; }

	bool IsOverlappingRect(const RECT& rc1, const RECT& rc2);

	D2D1_RECT_F ConvertRectToRectF(const RECT& rc);

	D2D1_COLOR_F ConvertColorrefToColorF(COLORREF color);
}