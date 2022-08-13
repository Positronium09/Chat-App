#include "pch.h"
#include "DirectWriteResources.h"

#pragma comment (lib, "dwrite.lib")


DirectWriteResources::DirectWriteResources()
{
	
}

HRESULT DirectWriteResources::InitializeResources()
{
	return DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory));
}

HRESULT DirectWriteResources::CreateTextFormat(const WCHAR* fontFamilyName)
{
	if (!factory)
	{
		return E_POINTER;
	}

	WCHAR* localeName = new WCHAR[LOCALE_NAME_MAX_LENGTH];
	GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

	HRESULT hr = factory->CreateTextFormat(fontFamilyName, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 
		14.0f, 
		localeName,
		&textFormat);

	if (FAILED(hr))
	{
		return hr;
	}

	ComPtr<IDWriteInlineObject> trimmingObject;
	factory->CreateEllipsisTrimmingSign(textFormat, &trimmingObject);

	DWRITE_TRIMMING trimming = {  };
	trimming.delimiter = 0;
	trimming.delimiterCount = 3;
	trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;

	textFormat->SetTrimming(&trimming, trimmingObject);

	textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	return hr;
}
