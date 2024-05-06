/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Graphics.h"

#ifdef BT_DEBUG
#define BT_GRAPHICS_DEBUG_ERROR_MESSAGES
#endif

namespace Berta
{
	Graphics::Graphics()
	{
	}

	Graphics::Graphics(const Size& size)
	{
		Build(size);
	}

	void Graphics::Build(const Size& size)
	{
		if (m_size != size)
		{
			m_size = size;
			if (m_size.IsEmpty())
			{
				Release();
				return;
			}

#ifdef BT_PLATFORM_WINDOWS
			HDC hdc = ::GetDC(nullptr);

			HDC cdc = ::CreateCompatibleDC(hdc);
			if (cdc == nullptr)
			{
				::ReleaseDC(nullptr, hdc);
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_TRACE << "Error." << std::endl;
#endif
				return;
			}

			HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, size.Width, size.Height);
			::SelectObject(cdc, hBitmap);

			::SetBkMode(cdc, TRANSPARENT);
			m_hdc = cdc;
			m_hBitmap = hBitmap;

			ReleaseDC(0, hdc);
#endif
		}
	}

	void Graphics::BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_hdc && graphicsSource.m_hdc)
		{
			if (!::BitBlt(m_hdc, rectDestination.X, rectDestination.Y, rectDestination.Width, rectDestination.Height, graphicsSource.m_hdc, pointSource.X, pointSource.Y, SRCCOPY))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << "BitBlt ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}
		}
#endif
	}

	void Graphics::DrawRectangle(const Color& color, bool solid)
	{
		DrawRectangle(m_size.ToRectangle(), color, solid);
	}

	void Graphics::DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		auto brush = ::CreateSolidBrush(color.RGB);
		RECT nativeRect{ static_cast<LONG>(rectangle.X), static_cast<LONG>(rectangle.Y), static_cast<LONG>(rectangle.X + rectangle.Width),static_cast<LONG>(rectangle.Y + rectangle.Height) };
		if (!FillRect(m_hdc, &nativeRect, brush))
		{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
			BT_CORE_ERROR << "FillRect ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
		}

		::DeleteObject(brush);
#endif
	}

	void Graphics::DrawString(const Point& position, const std::wstring& str, const Color& color)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_lastForegroundColor != color.RGB)
		{
			::SetTextColor(m_hdc, color.RGB);
			m_lastForegroundColor = color.RGB;
		}

		::TextOut(m_hdc, position.X, position.Y, str.c_str(), static_cast<int>(str.size()));
#endif
	}

	void Graphics::Paste(API::NativeWindowHandle destination, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destination, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::NativeWindowHandle destination, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_hdc)
		{
			HDC dc = ::GetDC(destination.Handle);
			if (dc)
			{
				if (!::BitBlt(dc, dx, dy, width, height, m_hdc, sx, sy, SRCCOPY))
				{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
					BT_CORE_ERROR << "BitBlt / Paste ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
				}

				::ReleaseDC(destination.Handle, dc);
			}
		}
#endif
	}

	void Graphics::Flush()
	{
#ifdef BT_PLATFORM_WINDOWS
		::GdiFlush();
#endif
	}

	void Graphics::Release()
	{
		m_size = Size::Zero;
	}
}