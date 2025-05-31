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
		/*HDC m_hdc{ nullptr };
		HBITMAP	m_hBitmap{ nullptr };
		ColorABGR* m_bmpColorBuffer{ nullptr };
		HFONT m_hFont{ nullptr };
		uint32_t m_bytesPerLine{ 0 };
		Size m_textExtent;*/

		ID2D1BitmapRenderTarget* m_bitmapRT{ nullptr };
		IDWriteTextFormat* m_textFormat{ nullptr };
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
		struct RootBufferNativeHandle
		{
#ifdef BT_PLATFORM_WINDOWS
			operator bool() const
			{
				return m_renderTarget != nullptr;
			}
			bool operator==(const RootBufferNativeHandle& other) const
			{
				return m_renderTarget == other.m_renderTarget;
			}

			bool operator!=(const RootBufferNativeHandle& other) const
			{
				return m_renderTarget != other.m_renderTarget;
			}

			ID2D1HwndRenderTarget* m_renderTarget{ nullptr };
#else
			operator bool() const
			{
				return false;
			}
			bool operator==(const RootBufferNativeHandle& other) const
			{
				return false;
			}

			bool operator!=(const RootBufferNativeHandle& other) const
			{
				return false;
			}
#endif

		};

		Size GetPaintHandleSize(PaintNativeHandle* handle);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::string& wstr);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr, size_t length);

		void Dispose(RootBufferNativeHandle& rootHandle);
	}
}

#endif