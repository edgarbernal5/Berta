/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PaintAPI.h"

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Platform/Windows/D2D.h"
#include <wrl/client.h>
#endif

namespace Berta
{
	PaintNativeHandle::~PaintNativeHandle()
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_bitmapRT)
		{
			m_bitmapRT->Release();
			m_bitmapRT = nullptr;
		}

		if (m_textFormat)
		{
			m_textFormat->Release();
			m_textFormat = nullptr;
		}
#endif
	}

	Size API::GetPaintHandleSize(PaintNativeHandle* handle)
	{
#ifdef BT_PLATFORM_WINDOWS
		//::BITMAP bmp;
		//::GetObject(handle->m_hBitmap, sizeof bmp, &bmp);

		//return Size(static_cast<uint32_t>(bmp.bmWidth), static_cast<uint32_t>(bmp.bmHeight));
		return {};
#else
		return {};
#endif
	}

	Size API::GetTextExtentSize(PaintNativeHandle* handle, const std::string& wstr)
	{
		return GetTextExtentSize(handle, StringUtils::Convert(wstr));
	}

	Size API::GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr)
	{
		return GetTextExtentSize(handle, wstr, wstr.size());
	}

	Size API::GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr, size_t length)
	{
#ifdef BT_PLATFORM_WINDOWS
		IDWriteTextLayout* textLayout = nullptr;
		
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			wstr.c_str(), 
			static_cast<UINT32>(length),
			handle->m_textFormat,
			FLT_MAX, FLT_MAX,
			&textLayout
		);

		if (SUCCEEDED(hr))
		{
			DWRITE_TEXT_METRICS metrics = {};
			textLayout->GetMetrics(&metrics);

			textLayout->Release();
			return { static_cast<uint32_t>(std::ceilf(metrics.widthIncludingTrailingWhitespace)), static_cast<uint32_t>(std::ceilf(metrics.height)) };
		}

		return {};
#else
		return {};
#endif
	}

	uint32_t API::GetCaretHeight(PaintNativeHandle* handle)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!handle->m_textFormat)
		{
			return 0;
		}

		WCHAR fontFamilyName[100] = {};
		handle->m_textFormat->GetFontFamilyName(fontFamilyName, ARRAYSIZE(fontFamilyName));

		Microsoft::WRL::ComPtr<IDWriteFontCollection> fontCollection;
		if (FAILED(handle->m_textFormat->GetFontCollection(&fontCollection)))
		{
			return 0;
		}

		UINT32 index = 0;
		BOOL exists = FALSE;
		if (FAILED(fontCollection->FindFamilyName(fontFamilyName, &index, &exists)) || !exists)
		{
			return 0;
		}

		Microsoft::WRL::ComPtr<IDWriteFontFamily> fontFamily;
		if (FAILED(fontCollection->GetFontFamily(index, &fontFamily)))
		{
			return 0;
		}

		Microsoft::WRL::ComPtr<IDWriteFont> font;
		if (FAILED(fontFamily->GetFirstMatchingFont(
			handle->m_textFormat->GetFontWeight(),
			handle->m_textFormat->GetFontStretch(),
			handle->m_textFormat->GetFontStyle(),
			&font)))
		{
			return 0;
		}

		DWRITE_FONT_METRICS metrics;
		font->GetMetrics(&metrics);

		float fontSize = handle->m_textFormat->GetFontSize();
		float ascent = static_cast<float>(metrics.ascent) * fontSize / metrics.designUnitsPerEm;
		float descent = static_cast<float>(metrics.descent) * fontSize / metrics.designUnitsPerEm;

		return static_cast<uint32_t>(ascent + descent);
#else
		return 0;
#endif
	}

	void API::Dispose(RootPaintNativeHandle& rootHandle)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (rootHandle)
		{
			rootHandle.RenderTarget->Release();
			rootHandle.RenderTarget = nullptr;
		}
#else
#endif
	}
}

