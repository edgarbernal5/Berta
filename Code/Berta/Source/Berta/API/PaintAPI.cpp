/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PaintAPI.h"

namespace Berta
{

	PaintNativeHandle::~PaintNativeHandle()
	{
		m_size = Size::Zero;

#ifdef BT_PLATFORM_WINDOWS
		if (m_hdc)
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
		}
#endif
	}

	Size API::GetPaintHandleSize(PaintNativeHandle* handle)
	{
#ifdef BT_PLATFORM_WINDOWS
		::BITMAP bmp;
		::GetObject(handle->m_hBitmap, sizeof bmp, &bmp);

		return Size(static_cast<uint32_t>(bmp.bmWidth), static_cast<uint32_t>(bmp.bmHeight));
#else
		return {};
#endif
	}
}

