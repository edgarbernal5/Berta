/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PAINT_API_HEADER
#define BT_PAINT_API_HEADER

#include <string>
#include "Berta/Core/Base.h"
#include "Berta/Core/BasicTypes.h"

namespace Berta
{
	struct PaintNativeHandle
	{
#ifdef BT_PLATFORM_WINDOWS
		HDC m_hdc{ nullptr };
		HBITMAP	m_hBitmap{ nullptr };
		ColorABGR* m_bmpColorBuffer{ nullptr };
		HFONT m_hFont{ nullptr };
		uint32_t m_bytesPerLine{ 0 };
		Size m_textExtent;
#else
#endif

		PaintNativeHandle() = default;
		~PaintNativeHandle();

		PaintNativeHandle(const PaintNativeHandle&) = delete;
		PaintNativeHandle& operator=(const PaintNativeHandle&) = delete;
	};

	namespace API
	{
		Size GetPaintHandleSize(PaintNativeHandle* handle);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::string& wstr);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr, size_t length);
	}
}

#endif