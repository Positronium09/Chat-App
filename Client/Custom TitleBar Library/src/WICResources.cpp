#include "pch.h"
#include "WICResources.h"


HRESULT WICResources::InitializeResources()
{
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));

	return hr;
}

HRESULT WICResources::GetDirect2DBitmapFromHICON(ID2D1RenderTarget* rt, HICON icon, ID2D1Bitmap** bmp)
{
	if (!factory || !bmp)
	{
		return E_POINTER;
	}

	*bmp = nullptr;

	ComPtr<IWICBitmap> wicBmp = nullptr;
	HRESULT hr = factory->CreateBitmapFromHICON(icon, &wicBmp);
	if (FAILED(hr)) { return hr; }

	ComPtr<IWICFormatConverter> converter;
	hr = factory->CreateFormatConverter(&converter);
	if (FAILED(hr)) { return hr; }

	hr = converter->Initialize(wicBmp, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) { return hr; }

	hr = rt->CreateBitmapFromWicBitmap(converter, bmp);

	return hr;
}
