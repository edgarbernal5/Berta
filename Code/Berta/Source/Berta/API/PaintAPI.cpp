/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PaintAPI.h"

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Platform/Windows/D2D.h"
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
			return { static_cast<uint32_t>(std::ceilf(metrics.width)), static_cast<uint32_t>(std::ceilf(metrics.height)) };
		}

		return {};
#else
		return {};
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

