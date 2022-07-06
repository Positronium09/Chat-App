#include "pch.h"
#include "Direct2DResources.h"

#pragma comment (lib, "d2d1.lib")


Direct2DResources::Direct2DResources()
{
	InitializeRenderTarget();
}

HRESULT Direct2DResources::InitializeResources()
{
	factory = nullptr;

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

	return hr;
}

HRESULT Direct2DResources::InitializeRenderTarget()
{
	if (!factory)
	{
		return E_POINTER;
	}

	auto renderTargetProps = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));

	return factory->CreateDCRenderTarget(&renderTargetProps, &rt);
}
