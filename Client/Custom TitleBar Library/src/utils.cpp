#include "pch.h"
#include "utils.h"


bool TitleBarLibrary::IsOverlappingRect(const RECT& rc1, const RECT& rc2)
{
	RECT intersRect = {  };

	IntersectRect(&intersRect, &rc1, &rc2);
	return !IsRectEmpty(&intersRect);
}

D2D1_RECT_F TitleBarLibrary::ConvertRectToRectF(const RECT& rc)
{
	return D2D1::RectF(
		static_cast<FLOAT>(rc.left), static_cast<FLOAT>(rc.top),
		static_cast<FLOAT>(rc.right), static_cast<FLOAT>(rc.bottom));
}

D2D1_COLOR_F TitleBarLibrary::ConvertColorrefToColorF(COLORREF color)
{
	return D2D1::ColorF(RGB(GetBValue(color), GetGValue(color), GetRValue(color)));
}
