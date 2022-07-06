#pragma once

#include "pch.h"
#include <wincodec.h>
#include <d2d1_1.h>
#include <atlbase.h>

template <class T>
using ComPtr = CComPtr<T>;

class WICResources
{
	private:
	inline static ComPtr<IWICImagingFactory> factory = nullptr;

	WICResources() { };

	public:
	static HRESULT InitializeResources();

	static auto& GetFactory() { return factory; };
	static HRESULT GetDirect2DBitmapFromHICON(ID2D1RenderTarget* rt, HICON icon, ID2D1Bitmap** bmp);
};
