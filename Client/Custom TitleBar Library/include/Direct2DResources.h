#pragma once

#include "pch.h"
#include <d2d1_1.h>
#include <atlbase.h>

template <class T>
using ComPtr = CComPtr<T>;


class Direct2DResources
{
	private:
	inline static ComPtr<ID2D1Factory> factory = nullptr;
	ComPtr<ID2D1DCRenderTarget> rt = nullptr;

	public:
	Direct2DResources();
	static HRESULT InitializeResources();
	HRESULT InitializeRenderTarget();

	static auto& GetFactory() { return factory; };
	auto& GetRenderTarget() { return rt; };
};
