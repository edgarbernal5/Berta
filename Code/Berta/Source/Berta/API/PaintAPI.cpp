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
			10000.0f, 10000.0f,
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
	
	Size API::GetTextExtentSize(PaintNativeHandle* handle, const Rectangle& area, const std::wstring& wstr)
	{
		return GetTextExtentSize(handle, area, wstr, wstr.size());
	}

	Size API::GetTextExtentSize(PaintNativeHandle* handle, const Rectangle& area, const std::string& str)
	{
		return GetTextExtentSize(handle, area, StringUtils::Convert(str), str.size());
	}

	Size API::GetTextExtentSize(PaintNativeHandle* handle, const Rectangle& area, const std::wstring& wstr, size_t length)
	{
#ifdef BT_PLATFORM_WINDOWS
		IDWriteTextLayout* textLayout = nullptr;
		
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			wstr.c_str(), 
			static_cast<UINT32>(length),
			handle->m_textFormat,
			static_cast<FLOAT>(area.Width), static_cast<FLOAT>(area.Height),
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
	
	Size API::GetTextExtentSize(PaintNativeHandle* handle, std::wstring_view wstr)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (wstr.empty()) return {};

		IDWriteTextLayout* textLayout = nullptr;
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			wstr.data(), 
			static_cast<UINT32>(wstr.size()),
			handle->m_textFormat,
			FLT_MAX, FLT_MAX,
			&textLayout
		);

		if (SUCCEEDED(hr))
		{
			DWRITE_TEXT_METRICS metrics = {};
			textLayout->GetMetrics(&metrics);
			textLayout->Release();
			
			return { static_cast<uint32_t>(std::ceilf(metrics.widthIncludingTrailingWhitespace)), 
					 static_cast<uint32_t>(std::ceilf(metrics.height)) };
		}
#endif
		return {};
	}

	uint32_t API::GetCaretHeight(PaintNativeHandle* handle)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!handle || !handle->m_textFormat)
		{
			return 0;
		}
		if (handle->m_metricsCached)
		{
			return static_cast<uint32_t>(handle->m_lineHeight);
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
		handle->m_ascent = static_cast<float>(metrics.ascent) * fontSize / metrics.designUnitsPerEm;
		float descent = static_cast<float>(metrics.descent) * fontSize / metrics.designUnitsPerEm;
		float lineGap = static_cast<float>(metrics.lineGap) * fontSize / metrics.designUnitsPerEm;

		handle->m_lineHeight = std::ceil(handle->m_ascent + descent + lineGap);
		handle->m_metricsCached = true;
		
		return static_cast<uint32_t>(handle->m_lineHeight);
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

