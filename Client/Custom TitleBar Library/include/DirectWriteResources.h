#pragma once

#include "pch.h"
#include <dwrite.h>
#include <atlbase.h>

template <class T>
using ComPtr = CComPtr<T>;


class DirectWriteResources
{
	private:
	inline static ComPtr<IDWriteFactory> factory = nullptr;
	ComPtr<IDWriteTextFormat> textFormat = nullptr;

	public:
	DirectWriteResources();
	static HRESULT InitializeResources();
	HRESULT CreateTextFormat(const WCHAR* fontFamilyName);

	static auto& GetFactory() { return factory; }
	auto& GetTextFormat() { return textFormat; }
};
