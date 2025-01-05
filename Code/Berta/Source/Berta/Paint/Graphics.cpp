/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Graphics.h"

#include <iostream>
#include <cmath>

#if BT_PLATFORM_WINDOWS
#pragma comment(lib, "Msimg32.lib") // added for AlphaBlend
#endif

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

	void Graphics::Blend(const Rectangle& blendRectangle, const Graphics& graphicsSource, const Point& pointSource, float alpha)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc && graphicsSource.m_attributes->m_hdc)
		{
			// Set up alpha blending
			BLENDFUNCTION blendFunc;
			blendFunc.BlendOp = AC_SRC_OVER;
			blendFunc.BlendFlags = 0;
			blendFunc.SourceConstantAlpha = static_cast<BYTE>(alpha * 255);
			blendFunc.AlphaFormat = 0;

			auto sourceSize = graphicsSource.m_attributes->m_size;

			// Apply alpha blending
			if (!::AlphaBlend(m_attributes->m_hdc, blendRectangle.X, blendRectangle.Y,
				(int)(blendRectangle.Width), (int)(blendRectangle.Height), graphicsSource.m_attributes->m_hdc,
				pointSource.X, pointSource.Y,
				sourceSize.Width, sourceSize.Height, blendFunc))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << " Graphics / AlphaBlend ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}
		}
#endif
	}

	void Graphics::BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc && graphicsSource.m_attributes->m_hdc)
		{
			if (!::BitBlt(m_attributes->m_hdc, rectDestination.X, rectDestination.Y, rectDestination.Width, rectDestination.Height, graphicsSource.m_attributes->m_hdc, pointSource.X, pointSource.Y, SRCCOPY))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << " Graphics / BitBlt ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}
		}
#endif
	}

	void Graphics::DrawLine(const Point& point1, const Point& point2, const Color& color, LineStyle style)
	{
		DrawLine(point1, point2, 1, color, style);
	}

	void Graphics::DrawLine(const Point& point1, const Point& point2, int lineWidth, const Color& color, LineStyle style)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		if (style == LineStyle::Dotted)
		{
			int dx = point2.X - point1.X;
			int dy = point2.Y - point1.Y;

			int absDx = abs(dx);
			int absDy = abs(dy);

			int steps = max(absDx, absDy);

			float xInc = dx / static_cast<float>(steps);
			float yInc = dy / static_cast<float>(steps);

			float x = static_cast<float>(point1.X);
			float y = static_cast<float>(point1.Y);

			auto stepsWidth = steps / lineWidth;
			for (int i = 0; i <= stepsWidth; ++i)
			{
				if (i % 2 == 0)
				{
					int xx = static_cast<int>(x + 0.5f);
					int yy = static_cast<int>(y + 0.5f);
					for (int ii = 0; ii < lineWidth; ii++)
					{
						for (int jj = 0; jj < lineWidth; jj++)
						{
							::SetPixel(m_attributes->m_hdc, xx + jj, yy + ii, color);
						}
					}
				}
				x += xInc * lineWidth;
				y += yInc * lineWidth;
			}
		}
		else
		{
			HPEN hPen = ::CreatePen(style == LineStyle::Solid ? PS_SOLID : (style == LineStyle::Dash ? PS_DASH : PS_DOT), lineWidth, color);

			HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

			::MoveToEx(m_attributes->m_hdc, point1.X, point1.Y, 0);
			::LineTo(m_attributes->m_hdc, point2.X, point2.Y);

			::SelectObject(m_attributes->m_hdc, hOldPen);
			::DeleteObject(hPen);
		}
#endif
	}

	void Graphics::DrawBeginLine(const Point& point, const Color& color, LineStyle style)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_hdc)
		{
			HPEN hPen = ::CreatePen(style == LineStyle::Solid ? PS_SOLID : (style == LineStyle::Dash ? PS_DASH : PS_DOT), 1, color);
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

		auto brush = ::CreateSolidBrush(color);
		RECT nativeRect = rectangle.ToRECT();
		if (solid)
		{
			if (!::FillRect(m_attributes->m_hdc, &nativeRect, brush))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << " Graphics / FillRect ::GetLastError() = " << ::GetLastError() << std::endl;
#endif
			}
		}
		else
		{
			if (!::FrameRect(m_attributes->m_hdc, &nativeRect, brush))
			{
#ifdef BT_GRAPHICS_DEBUG_ERROR_MESSAGES
				BT_CORE_ERROR << " Graphics / FrameRect ::GetLastError() = " << ::GetLastError() << std::endl;
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
		if (m_attributes->m_lastForegroundColor != color)
		{
			::SetTextColor(m_attributes->m_hdc, color);
			m_attributes->m_lastForegroundColor = color;
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
		if (m_attributes->m_lastForegroundColor != color)
		{
			::SetTextColor(m_attributes->m_hdc, color);
			m_attributes->m_lastForegroundColor = color;
		}
		::TextOut(m_attributes->m_hdc, position.X, position.Y, wstr.c_str(), static_cast<int>(wstr.size()));
		::SelectObject(m_attributes->m_hdc, oldFont);
#endif
	}
	
	//// Function to enable anti-aliasing for the given device context
	//void  Graphics::EnableAntiAliasing(HDC hdc) {
	//	auto aa=GetGraphicsMode(hdc);
	//	auto bb=SetGraphicsMode(hdc, GM_ADVANCED);  // Set the graphics mode to advanced
	//	SetBkMode(hdc, TRANSPARENT);         // Make sure background is transparent (optional)
	//	
	//	// Create a quality pen that supports anti-aliasing
	//	SetStretchBltMode(hdc, HALFTONE);   // Enable high-quality stretching (optional)
	//	SetTextCharacterExtra(hdc, 0);      // Prevent text distortion (optional)
	//}
	
	//Version 1
	/*
	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, const Color& color, ArrowDirection direction, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		Point arrowPoints[3];

		int centerX = (rect.X * 2 + (int)rect.Width) >> 1;
		int centerY = (rect.Y * 2 + (int)rect.Height) >> 1;

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
		else if (direction == ArrowDirection::Left)
		{
			arrowPoints[0].X = centerX + arrowLength;
			arrowPoints[0].Y = centerY + arrowWidth;

			arrowPoints[1].X = centerX + arrowLength;
			arrowPoints[1].Y = centerY - arrowWidth;

			arrowPoints[2].X = centerX - arrowLength;
			arrowPoints[2].Y = centerY;
		}

		HBRUSH brush = ::CreateSolidBrush(color);
		::SelectObject(m_attributes->m_hdc, brush);

		if (solid)
		{
			::BeginPath(m_attributes->m_hdc);
		}

		// Move to the first point
		HPEN hPen = ::CreatePen(PS_SOLID, 1, color);
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
	}*/
	
	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor)
	{
		DrawArrow(rect, arrowLength, arrowWidth, direction, borderColor, false, borderColor);
	}

//Version 2
	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor, bool solid, const Color& solidColor)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		POINT arrowPoints[4]{};

		int centerX = (rect.X * 2 + (int)rect.Width) >> 1;
		int centerY = (rect.Y * 2 + (int)rect.Height) >> 1;

		if (direction == ArrowDirection::Downwards)
		{
			arrowPoints[0].x = centerX - arrowWidth;
			arrowPoints[0].y = centerY - arrowLength;

			arrowPoints[1].x = centerX + arrowWidth;
			arrowPoints[1].y = centerY - arrowLength;

			arrowPoints[2].x = centerX;
			arrowPoints[2].y = centerY + arrowLength;
		}
		else if (direction == ArrowDirection::Upwards)
		{
			arrowPoints[0].x = centerX - arrowWidth;
			arrowPoints[0].y = centerY + arrowLength;

			arrowPoints[1].x = centerX + arrowWidth;
			arrowPoints[1].y = centerY + arrowLength;

			arrowPoints[2].x = centerX;
			arrowPoints[2].y = centerY - arrowLength;
		}
		else if (direction == ArrowDirection::Right)
		{
			arrowPoints[0].x = centerX - arrowLength;
			arrowPoints[0].y = centerY - arrowWidth;

			arrowPoints[1].x = centerX - arrowLength;
			arrowPoints[1].y = centerY + arrowWidth;

			arrowPoints[2].x = centerX + arrowLength;
			arrowPoints[2].y = centerY;
		}
		else if (direction == ArrowDirection::Left)
		{
			arrowPoints[0].x = centerX + arrowLength;
			arrowPoints[0].y = centerY + arrowWidth;

			arrowPoints[1].x = centerX + arrowLength;
			arrowPoints[1].y = centerY - arrowWidth;

			arrowPoints[2].x = centerX - arrowLength;
			arrowPoints[2].y = centerY;
		}

		arrowPoints[3] = arrowPoints[0];

		HPEN hPen = ::CreatePen(PS_SOLID, 1, borderColor);
		HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

		if (solid)
		{
			HBRUSH hBrush = ::CreateSolidBrush(solidColor);
			HBRUSH oldBrush = (HBRUSH)::SelectObject(m_attributes->m_hdc, hBrush);
			::Polygon(m_attributes->m_hdc, arrowPoints, 3);
			::DeleteObject(hBrush);
			::SelectObject(m_attributes->m_hdc, oldBrush);
		}
		else
		{
			::Polyline(m_attributes->m_hdc, arrowPoints, 4);
		}

		::DeleteObject(hPen);
		::SelectObject(m_attributes->m_hdc, hOldPen);
#endif
	}
	
	void Graphics::DrawRoundRectBox(const Rectangle& rect, const Color& color, const Color& bordercolor, bool solid)
	{
		DrawRoundRectBox(rect, 3, color, bordercolor, solid);
	}

	void Graphics::DrawRoundRectBox(const Rectangle& rect, int radius, const Color& color, const Color& bordercolor, bool solid)
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
			auto prevPen = ::SelectObject(m_attributes->m_hdc, ::CreatePen(PS_SOLID, 1, bordercolor));
			auto prevBrush = ::SelectObject(m_attributes->m_hdc, ::CreateSolidBrush(color));

			::RoundRect(m_attributes->m_hdc, rect.X, rect.Y, rect.X + static_cast<int>(rect.Width), rect.Y + static_cast<int>(rect.Height), static_cast<int>(radiusScaled * 2), static_cast<int>(radiusScaled * 2));

			::DeleteObject(::SelectObject(m_attributes->m_hdc, prevBrush));
			::DeleteObject(::SelectObject(m_attributes->m_hdc, prevPen));
		}
		else
		{
			auto brush = ::CreateSolidBrush(color);

			auto region = ::CreateRoundRectRgn(rect.X, rect.Y, rect.X + static_cast<int>(rect.Width) + 1, rect.Y + static_cast<int>(rect.Height) + 1, static_cast<int>(radiusScaled + 1), static_cast<int>(radiusScaled + 1));

			::FrameRgn(m_attributes->m_hdc, region, brush, 1, 1);

			::DeleteObject(region);
			::DeleteObject(brush);
		}
#endif
	}

	void Graphics::DrawGradientFill(const Rectangle& rect, const Color& startColor, const Color& endColor)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}
		auto x = (LONG)rect.X;
		auto y = (LONG)rect.Y;
		auto width = (LONG)rect.Width;
		auto height = (LONG)rect.Height;

		TRIVERTEX vertices[] =
		{
			{ x, y, (COLOR16)(GetRValue(startColor) << 8), (COLOR16)(GetGValue(startColor) << 8), (COLOR16)(GetBValue(startColor) << 8), (COLOR16)0xFF00 },
			{ x + width, y + height, (COLOR16)(GetRValue(endColor) << 8), (COLOR16)(GetGValue(endColor) << 8), (COLOR16)(GetBValue(endColor) << 8), (COLOR16)0xFF00 }
		};

		GRADIENT_RECT gradientRect = { 0, 1 };
		::GradientFill(m_attributes->m_hdc, vertices, 2, &gradientRect, 1, GRADIENT_FILL_RECT_V);
#endif
	}

	void Graphics::DrawButton(const Rectangle& rect, const Color& startColor, const Color& endColor, const Color& borderColor)
	{
#ifdef BT_PLATFORM_WINDOWS
		auto& hdc = m_attributes->m_hdc;
		int radius = 3;

		float scaleFactor = LayoutUtils::CalculateDPIScaleFactor(m_dpi);
		auto radiusScaled = static_cast<int>(radius * scaleFactor);

		int x = rect.X;
		int y = rect.Y;
		int height = rect.Height;
		int width = rect.Width;
		HRGN buttonRegion = ::CreateRoundRectRgn(x, y, x + width, y + height, radiusScaled, radiusScaled);
		::SelectClipRgn(hdc, buttonRegion);

		DrawGradientFill(rect, startColor, endColor);
		
		::SelectClipRgn(hdc, nullptr);
		//SetBkMode(hdc, TRANSPARENT);
		DrawRoundRectBox(rect, radius, borderColor, borderColor, false);
		::DeleteObject(buttonRegion);
#endif
	}

	void Graphics::DrawRoundedRectWithShadow(const Rectangle& rect, int radius, int shadowSize)
	{
		auto& hdc = m_attributes->m_hdc;

		float scaleFactor = LayoutUtils::CalculateDPIScaleFactor(m_dpi);
		auto radiusScaled = static_cast<int>(radius * scaleFactor);

		int x = rect.X;
		int y = rect.Y;
		int height = rect.Height;
		int width = rect.Width;

		// Paleta de colores de sombra (de más oscuro a más claro)
		COLORREF shadowColors[] = {
			RGB(50, 50, 50),  // Sombra oscura
			RGB(80, 80, 80),
			RGB(110, 110, 110),
			RGB(140, 140, 140),
			RGB(170, 170, 170),  // Sombra clara
		};

		int numShadows = sizeof(shadowColors) / sizeof(shadowColors[0]);

		// Dibuja sombras en capas alrededor del rectángulo central
		for (int i = 0; i < numShadows; i++) {
			HPEN hPen = CreatePen(PS_SOLID, 1, shadowColors[i]);
			HBRUSH hBrush = CreateSolidBrush(shadowColors[i]);
			SelectObject(hdc, hPen);
			SelectObject(hdc, hBrush);

			// Ajusta el tamaño de cada capa para crear la sombra difusa
			RoundRect(
				hdc,
				x - shadowSize + i,
				y - shadowSize + i,
				x + width + shadowSize - i,
				y + height + shadowSize - i,
				radiusScaled + (shadowSize - i),  // Ajusta el radio para mantener la curva
				radiusScaled + (shadowSize - i)
			);

			DeleteObject(hPen);
			DeleteObject(hBrush);
		}

		// Dibuja el rectángulo redondeado principal
		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));      // Color del borde
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));  // Color de relleno
		SelectObject(hdc, hPen);
		SelectObject(hdc, hBrush);

		RoundRect(hdc, x, y, x + width, y + height, radiusScaled, radiusScaled);

		DeleteObject(hPen);
		DeleteObject(hBrush);
	}

	void Graphics::DrawCircle(const Point& dest, int radius, const Color& fillColor, const Color& borderColor, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		HBRUSH hFillBrush = NULL;
		if (solid)
		{
			hFillBrush = ::CreateSolidBrush(fillColor);
		}
		HPEN hBorderPen = ::CreatePen(PS_SOLID, 1, borderColor);
		HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hBorderPen);

		HBRUSH hOldBrush = NULL;
		if (solid)
		{
			hOldBrush = (HBRUSH)::SelectObject(m_attributes->m_hdc, hFillBrush);
		}
		else
		{
			HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
			hOldBrush = (HBRUSH)::SelectObject(m_attributes->m_hdc, hNullBrush);
		}

		RECT destRECT{};
		destRECT.left = static_cast<LONG>(dest.X - radius);
		destRECT.top = static_cast<LONG>(dest.Y - radius);
		destRECT.right = static_cast<LONG>(dest.X + radius);
		destRECT.bottom = static_cast<LONG>(dest.Y + radius);

		::Ellipse(m_attributes->m_hdc, destRECT.left, destRECT.top, destRECT.right, destRECT.bottom);

		::SelectObject(m_attributes->m_hdc, hOldPen);
		::SelectObject(m_attributes->m_hdc, hOldBrush);

		if (solid)
		{
			::DeleteObject(hFillBrush);
		}
		::DeleteObject(hBorderPen);
#endif
	}

	void Graphics::DrawEllipse(const Rectangle& dest, const Color& fillColor, const Color& borderColor, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_hdc)
		{
			return;
		}

		HBRUSH hFillBrush = NULL;
		if (solid)
		{
			hFillBrush = ::CreateSolidBrush(fillColor);
		}
		HPEN hBorderPen = ::CreatePen(PS_SOLID, 1, borderColor);
		HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hBorderPen);

		HBRUSH hOldBrush = NULL;
		if (solid)
		{
			hOldBrush = (HBRUSH)::SelectObject(m_attributes->m_hdc, hFillBrush);
		}
		else
		{
			HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
			hOldBrush = (HBRUSH)::SelectObject(m_attributes->m_hdc, hNullBrush);
		}

		RECT destRECT{};
		destRECT.left = static_cast<LONG>(dest.X);
		destRECT.top = static_cast<LONG>(dest.Y);
		destRECT.right = static_cast<LONG>(dest.X + dest.Width);
		destRECT.bottom = static_cast<LONG>(dest.Y + dest.Height);

		::Ellipse(m_attributes->m_hdc, destRECT.left, destRECT.top, destRECT.right, destRECT.bottom);
		
		::SelectObject(m_attributes->m_hdc, hOldPen);
		::SelectObject(m_attributes->m_hdc, hOldBrush);

		if (solid)
		{
			::DeleteObject(hFillBrush);
		}
		::DeleteObject(hBorderPen);
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
				BT_CORE_ERROR << " Graphics / BitBlt-Paste ::GetLastError() = " << ::GetLastError() << std::endl;
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