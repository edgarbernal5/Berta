/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Graphics.h"
#include <iostream>

#ifdef BT_DEBUG
#define BT_GRAPHICS_DEBUG_ERROR_MESSAGES
#endif

namespace Berta
{
	Graphics::Graphics() :
		m_attributes(std::make_unique<NativeAttributes>())
	{
	}

	Graphics::Graphics(const Size& size) :
		m_attributes(std::make_unique<NativeAttributes>())
	{
		Build(size);
	}

	Graphics::Graphics(const Graphics& other) :
		m_attributes(std::make_unique<NativeAttributes>())
	{
	}

	Graphics::Graphics(Graphics&& other) noexcept :
		m_attributes(std::move(other.m_attributes))
	{
	}

	Graphics::~Graphics()
	{
		Release();
	}

	void Graphics::Build(const Size& size)
	{
		if (m_attributes->m_size != size)
		{
			m_attributes->m_size = size;
			if (m_attributes->m_size.IsEmpty())
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
			m_attributes->m_hdc = cdc;
			m_attributes->m_hBitmap = hBitmap;

			::LOGFONT lfText = {};
			SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, ::GetDpiForSystem());
			m_attributes->m_hFont = ::CreateFontIndirect(&lfText);
			if (m_attributes->m_hFont)
			{
			}

			//int dpi = GetDpiForSystem();
			//int baseFontSize = 20;
			//int scaledFontSize = MulDiv(baseFontSize, dpi, 96);

			//m_hFont = CreateTransparentFont(scaledFontSize, FW_NORMAL, false, false);

			::ReleaseDC(0, hdc);
#endif
		}
	}

	void Graphics::BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc && graphicsSource.m_attributes->m_hdc)
		{
			if (!::BitBlt(m_attributes->m_hdc, rectDestination.X, rectDestination.Y, rectDestination.Width, rectDestination.Height, graphicsSource.m_attributes->m_hdc, pointSource.X, pointSource.Y, SRCCOPY))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << "BitBlt ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}
		}
#endif
	}

	void Graphics::DrawLine(const Point& point1, const Point& point2, const Color& color)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc)
		{
			HPEN hPen = ::CreatePen(PS_SOLID, 1, color.RGB);
			HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

			::MoveToEx(m_attributes->m_hdc, point1.X, point1.Y, 0);
			::LineTo(m_attributes->m_hdc, point2.X, point2.Y);

			::SelectObject(m_attributes->m_hdc, hOldPen);
			::DeleteObject(hPen);
		}
#endif
	}

	void Graphics::DrawRectangle(const Color& color, bool solid)
	{
		DrawRectangle(m_attributes->m_size.ToRectangle(), color, solid);
	}

	void Graphics::DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		auto brush = ::CreateSolidBrush(color.RGB);
		RECT nativeRect = rectangle.ToRECT();
		if (solid)
		{
			if (!::FillRect(m_attributes->m_hdc, &nativeRect, brush))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << "FillRect ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}
		}
		else
		{
			if (!::FrameRect(m_attributes->m_hdc, &nativeRect, brush))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << "FrameRect ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}
		}

		::DeleteObject(brush);
#endif
	}

	void Graphics::DrawString(const Point& position, const std::wstring& str, const Color& color)
	{
		if (str.size() == 0)
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		HFONT oldFont = (HFONT)::SelectObject(m_attributes->m_hdc, m_attributes->m_hFont);
		if (m_attributes->m_lastForegroundColor != color.RGB)
		{
			::SetTextColor(m_attributes->m_hdc, color.RGB);
			m_attributes->m_lastForegroundColor = color.RGB;
		}
		::TextOut(m_attributes->m_hdc, position.X, position.Y, str.c_str(), static_cast<int>(str.size()));
		::SelectObject(m_attributes->m_hdc, oldFont);
#endif
	}

	void Graphics::Paste(API::NativeWindowHandle destination, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destination, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::NativeWindowHandle destination, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc)
		{
			HDC dc = ::GetDC(destination.Handle);
			if (dc)
			{
				if (!::BitBlt(dc, dx, dy, width, height, m_attributes->m_hdc, sx, sy, SRCCOPY))
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

	void Graphics::Swap(Graphics& other)
	{
		m_attributes.swap(other.m_attributes);
	}

	Size Graphics::GetStringSize(std::wstring& wstr)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc == nullptr || wstr.size() == 0)
			return {};

		::SIZE nativeSize;
		if (::GetTextExtentPoint32(m_attributes->m_hdc, wstr.c_str(), static_cast<int>(wstr.size()), &nativeSize))
		{
			return Size(nativeSize.cx, nativeSize.cy);
		}

		return {};
#else
		return {};
#endif
	}

	void Graphics::Release()
	{
		m_attributes.reset();
	}

	HFONT Graphics::CreateTransparentFont(int height, int weight, bool italic, bool underline)
	{
		::LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT)); // Clear the structure
		lf.lfHeight = height; // Font height
		lf.lfWeight = weight; // Font weight (bold)
		lf.lfItalic = italic; // Italic style
		lf.lfCharSet = DEFAULT_CHARSET; // Character set
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS; // Output precision
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS; // Clipping precision
		lf.lfQuality = DEFAULT_QUALITY; // Output quality
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE; // Pitch and family
		lstrcpy(lf.lfFaceName, L"Segoe UI"); // Font name

		// Create the font using CreateFontIndirect
		return ::CreateFontIndirect(&lf);
	}

	Graphics::NativeAttributes::~NativeAttributes()
	{
#ifdef BT_PLATFORM_WINDOWS
		m_size = Size::Zero;

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
}