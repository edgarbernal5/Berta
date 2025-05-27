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
		/*if (m_hdc)
		{
			::DeleteDC(m_hdc);
			m_hdc = nullptr;
		}

		if (m_hBitmap)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = nullptr;
		}

		if (m_hFont)
		{
			::DeleteObject(m_hFont);
			m_hFont = nullptr;
		}*/

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
		IDWriteTextLayout* pTextLayout = nullptr;
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout(
			wstr.c_str(), length,
			handle->m_textFormat,
			FLT_MAX, FLT_MAX, // Allow layout to determine its size
			&pTextLayout
		);

		if (SUCCEEDED(hr))
		{
			DWRITE_TEXT_METRICS metrics = {};
			pTextLayout->GetMetrics(&metrics);

			pTextLayout->Release();
			return { static_cast<uint32_t>(metrics.width), static_cast<uint32_t>(metrics.height) };
		}

		return {};
		///*if (!handle->m_hdc || wstr.size() == 0)
		//{
		//	return {};
		//}*/

		//HFONT oldFont = (HFONT)::SelectObject(handle->m_hdc, handle->m_hFont);
		//::SIZE nativeSize;
		//if (::GetTextExtentPoint32(handle->m_hdc, wstr.c_str(), static_cast<int>(length), &nativeSize))
		//{
		//	::SelectObject(handle->m_hdc, oldFont);
		//	return Size(nativeSize.cx, nativeSize.cy);
		//}
		//::SelectObject(handle->m_hdc, oldFont);

		//return {};
#else
		return {};
#endif
	}
}

