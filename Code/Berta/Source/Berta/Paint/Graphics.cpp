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

	void Graphics::DrawRoundRectBox(const Rectangle& rect, const Color& color)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		float scaleFactor = m_dpi / 96.0f;
		auto radius = static_cast<int>(8u * scaleFactor);
		auto ellipseHeight = radius;
		auto ellipseWidth = radius;
		//SetGraphicsMode(m_attributes->m_hdc, GM_ADVANCED);
		//SetBkMode(m_attributes->m_hdc, TRANSPARENT);
		//SetStretchBltMode(m_attributes->m_hdc, HALFTONE); // Optional, for better stretching quality
		HBRUSH brush = ::CreateSolidBrush(color.BGR);
		::SelectObject(m_attributes->m_hdc, brush);

		HPEN hPen = ::CreatePen(PS_SOLID, 1, color.BGR);

		HGDIOBJ oldBrush = ::SelectObject(m_attributes->m_hdc, brush);
		HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

		int left = rect.X;
		int top = rect.Y;
		int right = left+rect.Width-1;
		int bottom = top+rect.Height-1;
		
		//// Draw top and bottom lines
		//MoveToEx(m_attributes->m_hdc, left + ellipseWidth, top, NULL);
		//LineTo(m_attributes->m_hdc, right - ellipseWidth, top);
		//MoveToEx(m_attributes->m_hdc, left + ellipseWidth, bottom, NULL);
		//LineTo(m_attributes->m_hdc, right - ellipseWidth, bottom);

		//// Draw left and right lines
		//MoveToEx(m_attributes->m_hdc, left, top + ellipseHeight, NULL);
		//LineTo(m_attributes->m_hdc, left, bottom - ellipseHeight);
		//MoveToEx(m_attributes->m_hdc, right, top + ellipseHeight, NULL);
		//LineTo(m_attributes->m_hdc, right, bottom - ellipseHeight);

		// Draw corner arcs
		//Arc(m_attributes->m_hdc, left, top, left + 2 * ellipseWidth, top + 2 * ellipseHeight, left + ellipseWidth, top + ellipseHeight, left + ellipseWidth, top);
		//Arc(m_attributes->m_hdc, right - 2 * ellipseWidth, top, right, top + 2 * ellipseHeight, right - ellipseWidth, top + ellipseHeight, right - ellipseWidth, top);
		//Arc(m_attributes->m_hdc, left, bottom - 2 * ellipseHeight, left + 2 * ellipseWidth, bottom, left + ellipseWidth, bottom - ellipseHeight, left + ellipseWidth, bottom);
		/*Arc(m_attributes->m_hdc, right - 2 * ellipseWidth, bottom - 2 * ellipseHeight, right, bottom, right - ellipseWidth, bottom - ellipseHeight, right - ellipseWidth, bottom);*/

		// Define the coordinates for the arcs
		int arcWidth = radius * 2;
		int arcHeight = radius * 2;

		// Top-left corner arc
		Arc(m_attributes->m_hdc, left, top, left + arcWidth, top + arcHeight, left + radius, top, left, top + radius);

		// Top-right corner arc
		Arc(m_attributes->m_hdc, right - arcWidth, top, right, top + arcHeight, right, top + radius, right - radius, top);

		// Bottom-right corner arc
		Arc(m_attributes->m_hdc, right - arcWidth, bottom - arcHeight, right, bottom, right - radius, bottom, right, bottom - radius);

		// Bottom-left corner arc
		Arc(m_attributes->m_hdc, left, bottom - arcHeight, left + arcWidth, bottom, left, bottom - radius, left + radius, bottom);

		// Draw the straight sides
		MoveToEx(m_attributes->m_hdc, left + radius, top, NULL);
		LineTo(m_attributes->m_hdc, right - radius, top);

		MoveToEx(m_attributes->m_hdc, right, top + radius, NULL);
		LineTo(m_attributes->m_hdc, right, bottom - radius);

		MoveToEx(m_attributes->m_hdc, right - radius, bottom, NULL);
		LineTo(m_attributes->m_hdc, left + radius, bottom);

		MoveToEx(m_attributes->m_hdc, left, bottom - radius, NULL);
		LineTo(m_attributes->m_hdc, left, top + radius);


		//MyRoundRect(m_attributes->m_hdc, rect.X, rect.Y, rect.X + rect.Width, rect.Y+rect.Height, size, size);
		//::RoundRect(m_attributes->m_hdc, rect.X, rect.Y, rect.X + rect.Width, rect.Y + rect.Height, size, size);
		
		//SetBkMode(m_attributes->m_hdc, TRANSPARENT);
		// Clean up
		::SelectObject(m_attributes->m_hdc, oldBrush);
		::SelectObject(m_attributes->m_hdc, hOldPen);
		::DeleteObject(brush);
		::DeleteObject(hPen);
#endif
	}

	void Graphics::Paste(API::NativeWindowHandle destination, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destination, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::NativeWindowHandle destination, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

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
	void Graphics::MyArc(HDC hdc, int left, int top, int right, int bottom, int startX, int startY, int endX, int endY)
	{
		// Calculate the center and radius of the ellipse
		int width = right - left;
		int height = bottom - top;
		int cx = left + width / 2;
		int cy = top + height / 2;
		float rx = width / 2.0f;
		float ry = height / 2.0f;

		// Start and end angles
		float startAngle = atan2f(startY - cy, startX - cx);
		float endAngle = atan2f(endY - cy, endX - cx);

		// Ensure the angles are positive
		if (startAngle < 0) startAngle += 2 * M_PI;
		if (endAngle < 0) endAngle += 2 * M_PI;

		// Determine the sweep angle
		float sweepAngle = endAngle - startAngle;
		if (sweepAngle < 0) sweepAngle += 2 * M_PI;

		// Approximate the arc using multiple Bezier curves
		int numSegments = ceil(sweepAngle / (M_PI / 4)); // Use 45-degree segments
		float angleIncrement = sweepAngle / numSegments;

		POINT prevPoint = { startX, startY };
		for (int i = 1; i <= numSegments; ++i) {
			float angle1 = startAngle + (i - 1) * angleIncrement;
			float angle2 = startAngle + i * angleIncrement;

			// Calculate the points on the ellipse
			float x1 = cx + rx * cosf(angle1);
			float y1 = cy + ry * sinf(angle1);
			float x2 = cx + rx * cosf(angle2);
			float y2 = cy + ry * sinf(angle2);

			// Calculate control points for the Bezier curve
			float ctrl1X = x1 - (rx / 4) * sinf(angle1);
			float ctrl1Y = y1 + (ry / 4) * cosf(angle1);
			float ctrl2X = x2 + (rx / 4) * sinf(angle2);
			float ctrl2Y = y2 - (ry / 4) * cosf(angle2);

			// Define the Bezier curve points
			POINT bezierPoints[] = { prevPoint, { static_cast<LONG>(ctrl1X), static_cast<LONG>(ctrl1Y) }, { static_cast<LONG>(ctrl2X), static_cast<LONG>(ctrl2Y) }, { static_cast<LONG>(x2), static_cast<LONG>(y2) } };
			MyPolyBezier(hdc, bezierPoints, 4);

			// Update the previous point
			prevPoint = { static_cast<LONG>(x2), static_cast<LONG>(y2) };
		}
	}
	void Graphics::MyPolyBezier(HDC hdc, const POINT* points, int numPoints)
	{
		if (numPoints < 4) {
			// Not enough points to draw a Bezier curve
			return;
		}

		const int NUM_STEPS = 100; // Number of steps to approximate the Bezier curve
		double t;
		POINT prevPoint = points[0];

		for (int i = 1; i < numPoints; i += 3) {
			for (int step = 1; step <= NUM_STEPS; ++step) {
				t = step / (double)NUM_STEPS;

				// Calculate the position of the Bezier curve at t
				double x = pow(1 - t, 3) * points[i - 1].x + 3 * pow(1 - t, 2) * t * points[i].x +
					3 * (1 - t) * pow(t, 2) * points[i + 1].x + pow(t, 3) * points[i + 2].x;

				double y = pow(1 - t, 3) * points[i - 1].y + 3 * pow(1 - t, 2) * t * points[i].y +
					3 * (1 - t) * pow(t, 2) * points[i + 1].y + pow(t, 3) * points[i + 2].y;

				POINT currPoint = { static_cast<LONG>(x), static_cast<LONG>(y) };
				MoveToEx(hdc, prevPoint.x, prevPoint.y, NULL);
				LineTo(hdc, currPoint.x, currPoint.y);

				prevPoint = currPoint;
			}
		}
	}
	void Graphics::MyRoundRect(HDC hdc, int left, int top, int right, int bottom, int width, int height)
	{
		// Draw the straight edges
		MoveToEx(hdc, left + width, top, NULL);
		LineTo(hdc, right - width, top);
		LineTo(hdc, right, top + height);
		LineTo(hdc, right, bottom - height);
		LineTo(hdc, right - width, bottom);
		LineTo(hdc, left + width, bottom);
		LineTo(hdc, left, bottom - height);
		LineTo(hdc, left, top + height);
		LineTo(hdc, left + width, top);

		// Draw the corner arcs using MyArc
		MyArc(hdc, left, top, left + width * 2, top + height * 2, left + width, top, left, top + height); // Top-left corner
		MyArc(hdc, right - width * 2, top, right, top + height * 2, right - width, top, right, top + height); // Top-right corner
		MyArc(hdc, left, bottom - height * 2, left + width * 2, bottom, left, bottom - height, left + width, bottom); // Bottom-left corner
		MyArc(hdc, right - width * 2, bottom - height * 2, right, bottom, right, bottom - height, right - width, bottom); // Bottom-right corner
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