/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Graphics.h"

#include <iostream>
#include <cmath>

// Define M_PI if it's not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
		if (!m_attributes)
		{
			m_attributes = std::make_unique<NativeAttributes>();
		}

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

			::ReleaseDC(0, hdc);
#endif
		}
	}

	void Graphics::BuildFont(uint32_t dpi)
	{
		m_dpi = dpi;
		if (!m_attributes || m_attributes->m_hdc == nullptr)
		{
			return;
		}

		if (m_attributes->m_hFont)
		{
			::DeleteObject(m_attributes->m_hFont);
			m_attributes->m_hFont = nullptr;
		}

		::LOGFONT lfText = {};
		SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, dpi);
		m_attributes->m_hFont = ::CreateFontIndirect(&lfText);

		HFONT oldFont = (HFONT)::SelectObject(m_attributes->m_hdc, m_attributes->m_hFont);
		m_attributes->m_textExtent = GetTextExtent(L"()[]{}");
		::SelectObject(m_attributes->m_hdc, oldFont);
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
		if (!m_attributes->m_hdc)
		{
			return;
		}

		HPEN hPen = ::CreatePen(PS_SOLID, 1, color.BGR);
		HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

		::MoveToEx(m_attributes->m_hdc, point1.X, point1.Y, 0);
		::LineTo(m_attributes->m_hdc, point2.X, point2.Y);

		::SelectObject(m_attributes->m_hdc, hOldPen);
		::DeleteObject(hPen);
#endif
	}

	void Graphics::DrawBeginLine(const Point& point, const Color& color)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc)
		{
			HPEN hPen = ::CreatePen(PS_SOLID, 1, color.BGR);
			HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

			::MoveToEx(m_attributes->m_hdc, point.X, point.Y, 0);
		}
#endif
	}

	void Graphics::DrawLineTo(const Point& point, const Color& color)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}
		::LineTo(m_attributes->m_hdc, point.X, point.Y);
#endif
	}

	void Graphics::DrawRectangle(const Color& color, bool solid)
	{
		DrawRectangle(m_attributes->m_size.ToRectangle(), color, solid);
	}

	void Graphics::DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		auto brush = ::CreateSolidBrush(color.BGR);
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
		if (m_attributes->m_lastForegroundColor != color.BGR)
		{
			::SetTextColor(m_attributes->m_hdc, color.BGR);
			m_attributes->m_lastForegroundColor = color.BGR;
		}
		::TextOut(m_attributes->m_hdc, position.X, position.Y, str.c_str(), static_cast<int>(str.size()));
		::SelectObject(m_attributes->m_hdc, oldFont);
#endif
	}

	void Graphics::DrawString(const Point& position, const std::string& str, const Color& color)
	{
		if (str.size() == 0)
		{
			return;
		}

		auto wstr = StringUtils::Convert(str);
#ifdef BT_PLATFORM_WINDOWS
		HFONT oldFont = (HFONT)::SelectObject(m_attributes->m_hdc, m_attributes->m_hFont);
		if (m_attributes->m_lastForegroundColor != color.BGR)
		{
			::SetTextColor(m_attributes->m_hdc, color.BGR);
			m_attributes->m_lastForegroundColor = color.BGR;
		}
		::TextOut(m_attributes->m_hdc, position.X, position.Y, wstr.c_str(), static_cast<int>(wstr.size()));
		::SelectObject(m_attributes->m_hdc, oldFont);
#endif
	}

	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, const Color& color, ArrowDirection direction, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		Point arrowPoints[3];

		int centerX = (rect.X * 2 + rect.Width) >> 1;
		int centerY = (rect.Y * 2 + rect.Height) >> 1;

		if (direction == ArrowDirection::Downwards)
		{
			arrowPoints[0].X = centerX - arrowWidth;
			arrowPoints[0].Y = centerY - arrowLength;

			arrowPoints[1].X = centerX + arrowWidth;
			arrowPoints[1].Y = centerY - arrowLength;

			arrowPoints[2].X = centerX;
			arrowPoints[2].Y = centerY + arrowLength;
		}
		else if (direction == ArrowDirection::Upwards)
		{
			arrowPoints[0].X = centerX - arrowWidth;
			arrowPoints[0].Y = centerY + arrowLength;

			arrowPoints[1].X = centerX + arrowWidth;
			arrowPoints[1].Y = centerY + arrowLength;

			arrowPoints[2].X = centerX;
			arrowPoints[2].Y = centerY - arrowLength;
		}
		else if (direction == ArrowDirection::Right)
		{
			arrowPoints[0].X = centerX - arrowLength;
			arrowPoints[0].Y = centerY - arrowWidth;

			arrowPoints[1].X = centerX - arrowLength;
			arrowPoints[1].Y = centerY + arrowWidth;

			arrowPoints[2].X = centerX + arrowLength;
			arrowPoints[2].Y = centerY;
		}

		HBRUSH brush = ::CreateSolidBrush(color.BGR);
		::SelectObject(m_attributes->m_hdc, brush);

		if (solid)
		{
			::BeginPath(m_attributes->m_hdc);
		}

		// Move to the first point
		HPEN hPen = ::CreatePen(PS_SOLID, 1, color.BGR);
		HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

		::MoveToEx(m_attributes->m_hdc, arrowPoints[0].X, arrowPoints[0].Y, 0);

		// Draw lines to each subsequent point
		for (int i = 1; i < 3; ++i)
		{
			::LineTo(m_attributes->m_hdc, arrowPoints[i].X, arrowPoints[i].Y);
		}

		// Close the polygon by drawing a line back to the first point
		::LineTo(m_attributes->m_hdc, arrowPoints[0].X, arrowPoints[0].Y);

		if (solid)
		{
			::EndPath(m_attributes->m_hdc);
			::FillPath(m_attributes->m_hdc);
		}

		if (hPen)
		{
			::DeleteObject(hPen);
		}

		if (brush)
		{
			::DeleteObject(brush);
		}
#endif
	}

	void Graphics::DrawRoundRectBox(const Rectangle& rect, const Color& color, bool solid)
	{
		DrawRoundRectBox(rect, 3, color, solid);
	}

	void Graphics::DrawRoundRectBox(const Rectangle& rect, int radius, const Color& color, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		float scaleFactor = LayoutUtils::CalculateDPIScaleFactor(m_dpi);
		auto radiusScaled = static_cast<int>(radius * scaleFactor);

		if (solid)
		{
			auto prv_pen = ::SelectObject(m_attributes->m_hdc, ::CreatePen(PS_SOLID, 1, color.BGR));
			auto prv_brush = ::SelectObject(m_attributes->m_hdc, ::CreateSolidBrush(0xFFFFFF));

			::RoundRect(m_attributes->m_hdc, rect.X, rect.Y, rect.X + rect.Width, rect.Y + rect.Height, static_cast<int>(radiusScaled * 2), static_cast<int>(radiusScaled * 2));

			::DeleteObject(::SelectObject(m_attributes->m_hdc, prv_brush));
			::DeleteObject(::SelectObject(m_attributes->m_hdc, prv_pen));
		}
		else
		{
			auto brush = ::CreateSolidBrush(color.BGR);

			auto region = ::CreateRoundRectRgn(rect.X, rect.Y, rect.X + static_cast<int>(rect.Width) + 1, rect.Y + static_cast<int>(rect.Height) + 1, static_cast<int>(radiusScaled + 1), static_cast<int>(radiusScaled + 1));

			::FrameRgn(m_attributes->m_hdc, region, brush, 1, 1);

			::DeleteObject(region);
			::DeleteObject(brush);
		}
#endif
	}

	void Graphics::Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destinationHandle, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::NativeWindowHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		HDC dc = ::GetDC(destinationHandle.Handle);
		if (dc)
		{
			if (!::BitBlt(dc, dx, dy, width, height, m_attributes->m_hdc, sx, sy, SRCCOPY))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << "BitBlt / Paste ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}

			::ReleaseDC(destinationHandle.Handle, dc);
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

	Size Graphics::GetTextExtent(const std::wstring& wstr)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc || wstr.size() == 0)
		{
			return {};
		}

		HFONT oldFont = (HFONT)::SelectObject(m_attributes->m_hdc, m_attributes->m_hFont);
		::SIZE nativeSize;
		if (::GetTextExtentPoint32(m_attributes->m_hdc, wstr.c_str(), static_cast<int>(wstr.size()), &nativeSize))
		{
			::SelectObject(m_attributes->m_hdc, oldFont);
			return Size(nativeSize.cx, nativeSize.cy);
		}
		::SelectObject(m_attributes->m_hdc, oldFont);

		return {};
#else
		return {};
#endif
	}

	Size Graphics::GetTextExtent(const std::string& str)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc == nullptr || str.size() == 0)
		{
			return {};
		}
		auto wstr = StringUtils::Convert(str);
		HFONT oldFont = (HFONT)::SelectObject(m_attributes->m_hdc, m_attributes->m_hFont);
		::SIZE nativeSize;
		if (::GetTextExtentPoint32(m_attributes->m_hdc, wstr.c_str(), static_cast<int>(wstr.size()), &nativeSize))
		{
			::SelectObject(m_attributes->m_hdc, oldFont);
			return Size(nativeSize.cx, nativeSize.cy);
		}

		::SelectObject(m_attributes->m_hdc, oldFont);
		return {};
#else
		return {};
#endif
	}

	Size Graphics::GetTextExtent(const std::wstring& wstr, size_t length)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc == nullptr || wstr.size() == 0)
		{
			return {};
		}

		HFONT oldFont = (HFONT)::SelectObject(m_attributes->m_hdc, m_attributes->m_hFont);
		::SIZE nativeSize;
		if (::GetTextExtentPoint32(m_attributes->m_hdc, wstr.c_str(), static_cast<int>(length), &nativeSize))
		{
			::SelectObject(m_attributes->m_hdc, oldFont);
			return Size(nativeSize.cx, nativeSize.cy);
		}

		::SelectObject(m_attributes->m_hdc, oldFont);
		return {};
#else
		return {};
#endif
	}

	void Graphics::Release()
	{
		m_attributes.reset();
	}

	Graphics::NativeAttributes::~NativeAttributes()
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
}