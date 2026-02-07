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

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Platform/Windows/D2D.h"
#include <wrl/client.h>
#endif

namespace Berta
{
	struct PaintNativeHandle
	{
#ifdef BT_PLATFORM_WINDOWS
		ID2D1BitmapRenderTarget* m_bitmapRT{ nullptr };
		IDWriteTextFormat* m_textFormat{ nullptr };
		Size m_textExtent;
		
		float m_lineHeight { 0 };
		float m_ascent { 0 };
		bool m_metricsCached { false };
#else
#endif

		PaintNativeHandle() = default;
		~PaintNativeHandle();

		PaintNativeHandle(const PaintNativeHandle&) = delete;
		PaintNativeHandle& operator=(const PaintNativeHandle&) = delete;
	};
	
	struct TextPaintNativeHandle
	{
#ifdef BT_PLATFORM_WINDOWS
		Microsoft::WRL::ComPtr<IDWriteTextLayout> m_textLayout{ nullptr };
		
		bool IsValid() const { return m_textLayout != nullptr; }
		operator bool() const
		{
			return m_textLayout != nullptr;
		}
		void Release()
		{
			m_textLayout.Reset();
		}
#else
		bool IsValid() const { return false; }
		operator bool() const
		{
			return false;
		}
		void Release()
		{
		}
#endif
		
	};

	namespace API
	{
		struct RootPaintNativeHandle
		{
#ifdef BT_PLATFORM_WINDOWS
			operator bool() const
			{
				return RenderTarget != nullptr;
			}

			bool operator==(const RootPaintNativeHandle& other) const
			{
				return RenderTarget == other.RenderTarget;
			}

			bool operator!=(const RootPaintNativeHandle& other) const
			{
				return RenderTarget != other.RenderTarget;
			}

			ID2D1HwndRenderTarget* RenderTarget{ nullptr };
#else
			operator bool() const
			{
				return false;
			}
			bool operator==(const RootPaintNativeHandle& other) const
			{
				return false;
			}

			bool operator!=(const RootPaintNativeHandle& other) const
			{
				return false;
			}
#endif

		};

		Size GetPaintHandleSize(PaintNativeHandle* handle);
		
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::string& wstr);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr);
		Size GetTextExtentSize(PaintNativeHandle* handle, const std::wstring& wstr, size_t length);
		Size GetTextExtentSize(PaintNativeHandle* handle, const Rectangle& area, const std::wstring& wstr);
		Size GetTextExtentSize(PaintNativeHandle* handle, const Rectangle& area, const std::string& str);
		Size GetTextExtentSize(PaintNativeHandle* handle, const Rectangle& area, const std::wstring& wstr, size_t length);
		Size GetTextExtentSize(PaintNativeHandle* handle, std::wstring_view wstr);
		
		uint32_t GetCaretHeight(PaintNativeHandle* handle);

		void Dispose(RootPaintNativeHandle& rootHandle);
	}
}

#endif