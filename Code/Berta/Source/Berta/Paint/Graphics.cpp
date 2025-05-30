/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Graphics.h"

#include "Berta/Paint/ColorBuffer.h"

#include <iostream>
#include <cmath>

#if BT_PLATFORM_WINDOWS
#include <comdef.h>
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
		m_attributes(new PaintNativeHandle())
	{
	}

	Graphics::Graphics(const Size& size, uint32_t dpi, API::RootBufferNativeHandle nativeHandle) :
		m_attributes(new PaintNativeHandle()),
		m_dpi(dpi)
	{
		Build(size, nativeHandle);
	}

	/*Graphics::Graphics(const Graphics& other)
	{
	}*/

	Graphics::Graphics(Graphics&& other) noexcept :
		m_attributes(std::move(other.m_attributes)),
		m_dpi(other.m_dpi),
		m_size(other.m_size),
		m_nativeWindowHandle(other.m_nativeWindowHandle)
	{
		other.m_attributes.reset(new PaintNativeHandle());
	}

	Graphics::~Graphics()
	{
		Release();
	}

	Graphics& Graphics::operator=(const Graphics& other)
	{
		if (this != &other)
		{
			/*m_attributes = std::move(other.m_attributes);
			m_dpi = std::move(other.m_dpi);
			m_size = std::move(other.m_size);
			m_renderTarget = std::move(other.m_renderTarget);
			m_bitmapRT = std::move(other.m_bitmapRT);*/
		}
		return *this;
	}

	Graphics& Graphics::operator=(Graphics&& other)
	{
		if (this != &other)
		{
			m_attributes = std::move(other.m_attributes);
			m_dpi = std::move(other.m_dpi);
			m_size = std::move(other.m_size);
		}

		return *this;
	}

	void Graphics::Build(const Size& size, API::RootBufferNativeHandle nativeWindowHandle)
	{
		if (m_size == size)
		{
			return;
		}

		m_size = size;
		if (m_size.IsEmpty())
		{
			Release();
			return;
		}

		if (!m_attributes)
		{
			m_attributes.reset(new PaintNativeHandle());
		}

		m_nativeWindowHandle = nativeWindowHandle;
#ifdef BT_PLATFORM_WINDOWS

		if (m_attributes->m_bitmapRT == nullptr)
		{
			D2D1_SIZE_F desiredSize = D2D1::SizeF(static_cast<FLOAT>(m_size.Width), static_cast<FLOAT>(m_size.Height));

			D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat
			(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_PREMULTIPLIED
			);

			HRESULT hr = m_nativeWindowHandle.m_renderTarget->CreateCompatibleRenderTarget
			(
				desiredSize, D2D1::SizeU(m_size.Width, m_size.Height), pixelFormat,
				&m_attributes->m_bitmapRT
			);

			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Error creating bitmap render target." << std::endl;
			}
		}

#endif
	}

	void Graphics::BuildFont(uint32_t dpi)
	{
		m_dpi = dpi;
		if (!m_attributes)
		{
			return;
		}
#ifdef BT_PLATFORM_WINDOWS

		::LOGFONT lfText = {};
		::SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, dpi);

		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextFormat(
			lfText.lfFaceName,
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			static_cast<FLOAT>(std::abs(lfText.lfHeight)),
			L"en-us",
			&m_attributes->m_textFormat
		);

		m_attributes->m_textExtent = GetTextExtent("{}[]");
#endif
	}

	void Graphics::Rebuild(const Size& size)
	{
		if (m_size == size)
		{
			return;
		}

		Release();
		Build(size, {});
	}

	void Graphics::Blend(const Rectangle& blendDestRectangle, const Graphics& graphicsSource, const Point& pointSource, double alpha)
	{
		/*Rectangle sourceRect;
		sourceRect.X = pointSource.X;
		sourceRect.Y = pointSource.Y;
		sourceRect.Width = blendDestRectangle.Width;
		sourceRect.Height = blendDestRectangle.Height;

		ColorBuffer destBuffer;
		destBuffer.Attach(graphicsSource.GetHandle(), sourceRect);

		destBuffer.Blend(sourceRect, m_attributes.get(), blendDestRectangle, 1.0 - alpha);*/
	}

	void Graphics::BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource)
	{
#ifdef BT_PLATFORM_WINDOWS
		ID2D1Bitmap* sourceBitmap = nullptr;
		if (SUCCEEDED(graphicsSource.m_attributes->m_bitmapRT->GetBitmap(&sourceBitmap)))
		{
			//D2D1_SIZE_F sourceSize = sourceBitmap->GetSize();
			auto sourceSize = graphicsSource.GetSize();
			D2D1_RECT_F destRect = D2D1::RectF(static_cast<FLOAT>(rectDestination.X), static_cast<FLOAT>(rectDestination.Y), static_cast<FLOAT>(rectDestination.X + sourceSize.Width), static_cast<FLOAT>(rectDestination.Y + sourceSize.Height));
			D2D1_RECT_F srcRect = D2D1::RectF(static_cast<FLOAT>(pointSource.X), static_cast<FLOAT>(pointSource.Y), static_cast<FLOAT>(pointSource.X + sourceSize.Width), static_cast<FLOAT>(pointSource.Y + sourceSize.Height));
			
			m_attributes->m_bitmapRT->DrawBitmap(sourceBitmap, destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
			sourceBitmap->Release();
		}
		else
		{
			BT_CORE_ERROR << " Graphics / BitBlt" << std::endl;
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
		if (!m_attributes)
		{
			return;
		}

		ID2D1SolidColorBrush* brush;
		auto hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(color, &brush);

		if (SUCCEEDED(hr))
		{
			D2D1_POINT_2F point1F;
			point1F.x = static_cast<FLOAT>(point1.X) + 0.5f;
			point1F.y = static_cast<FLOAT>(point1.Y) + 0.5f;

			D2D1_POINT_2F point2F;
			point2F.x = static_cast<FLOAT>(point2.X) + 0.5f;
			point2F.y = static_cast<FLOAT>(point2.Y) + 0.5f;

			if (style == LineStyle::Solid)
			{
				m_attributes->m_bitmapRT->DrawLine(point1F, point2F, brush);
			}
			else
			{
				ID2D1StrokeStyle* strokeStyle = nullptr;

				D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties(
					D2D1_CAP_STYLE_ROUND,      // startCap
					D2D1_CAP_STYLE_ROUND,      // endCap
					D2D1_CAP_STYLE_FLAT,       // dashCap
					D2D1_LINE_JOIN_ROUND,      // lineJoin
					10.0f,                     // miterLimit
					D2D1_DASH_STYLE_DASH,      // dashStyle
					0.0f                       // dashOffset
				);
				DirectX::D2DModule::GetInstance().GetFactory()->CreateStrokeStyle(&props, nullptr, 0, &strokeStyle);
				m_attributes->m_bitmapRT->DrawLine(point1F, point2F, brush, 1.0f, strokeStyle);

				strokeStyle->Release();
			}
			brush->Release();
		}

		/*if (style == LineStyle::Dotted)
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
		}*/
#endif
	}

	void Graphics::DrawBeginLine(const Point& point, const Color& color, LineStyle style)
	{
#ifdef BT_PLATFORM_WINDOWS
		/*if (m_attributes->m_hdc)
		{
			HPEN hPen = ::CreatePen(style == LineStyle::Solid ? PS_SOLID : (style == LineStyle::Dash ? PS_DASH : PS_DOT), 1, color);
			HPEN hOldPen = (HPEN)::SelectObject(m_attributes->m_hdc, hPen);

			::MoveToEx(m_attributes->m_hdc, point.X, point.Y, 0);
		}*/
#endif
	}

	void Graphics::DrawLineTo(const Point& point, const Color& color)
	{
#ifdef BT_PLATFORM_WINDOWS
		//if (!m_attributes->m_hdc)
		//{
		//	return;
		//}
		//::LineTo(m_attributes->m_hdc, point.X, point.Y);
#endif
	}

	void Graphics::DrawRectangle(const Color& color, bool solid)
	{
		DrawRectangle(m_size.ToRectangle(), color, solid);
	}

	void Graphics::DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		D2D1_RECT_F d2dRect = rectangle;

		ID2D1SolidColorBrush* brush;
		auto hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(color, &brush);

		if (SUCCEEDED(hr))
		{
			if (solid)
			{
				m_attributes->m_bitmapRT->FillRectangle(&d2dRect, brush);
			}
			else
			{
				m_attributes->m_bitmapRT->DrawRectangle(&d2dRect, brush);
			}

			brush->Release();
		}
#endif
	}

	void Graphics::DrawString(const Point& position, const std::wstring& wstr, const Color& color)
	{
		if (wstr.size() == 0)
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		auto textSize = GetTextExtent(wstr);

		D2D1_RECT_F d2dRect;
		d2dRect.left = static_cast<FLOAT>(position.X);
		d2dRect.top = static_cast<FLOAT>(position.Y);
		d2dRect.right = static_cast<FLOAT>(position.X + textSize.Width + 1);
		d2dRect.bottom = static_cast<FLOAT>(position.Y + textSize.Height + 1);

		ID2D1SolidColorBrush* brush;
		m_attributes->m_bitmapRT->CreateSolidColorBrush(color,
			&brush);

		m_attributes->m_bitmapRT->DrawText
		(
			wstr.c_str(),
			static_cast<UINT32>(wstr.size()),
			m_attributes->m_textFormat,
			d2dRect,
			brush
		);

		brush->Release();
#endif
	}

	void Graphics::DrawString(const Point& position, const std::string& str, const Color& color)
	{
		DrawString(position, StringUtils::Convert(str), color);
	}
	
	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor)
	{
		DrawArrow(rect, arrowLength, arrowWidth, direction, borderColor, false, borderColor);
	}

//Version 2
	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor, bool solid, const Color& solidColor)
	{
#ifdef BT_PLATFORM_WINDOWS
		
#endif
	}
	
	void Graphics::DrawRoundRectBox(const Rectangle& rect, const Color& color, const Color& bordercolor, bool solid)
	{
		DrawRoundRectBox(rect, 3, color, bordercolor, solid);
	}

	void Graphics::DrawRoundRectBox(const Rectangle& rect, int radius, const Color& color, const Color& bordercolor, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		float scaleFactor = LayoutUtils::CalculateDPIScaleFactor(m_dpi);
		auto radiusScaled = radius * scaleFactor;
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect
		(
			D2D1::RectF(static_cast<FLOAT>(rect.X) + 0.5f, static_cast<FLOAT>(rect.Y + 0.5f), 
				static_cast<FLOAT>(rect.X + rect.Width), static_cast<FLOAT>(rect.Y + rect.Height)),
			radiusScaled,
			radiusScaled
		);

		ID2D1SolidColorBrush* brush;
		auto hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(color, &brush);
		if (FAILED(hr))
		{
			return;
		}

		ID2D1SolidColorBrush* brushBorder;
		hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(bordercolor, &brushBorder);
		if (SUCCEEDED(hr))
		{
			if (solid)
			{
				m_attributes->m_bitmapRT->FillRoundedRectangle(&roundedRect, brush);
				m_attributes->m_bitmapRT->DrawRoundedRectangle(&roundedRect, brushBorder);
			}
			else
			{

				m_attributes->m_bitmapRT->DrawRoundedRectangle(&roundedRect, brushBorder);
			}
			brushBorder->Release();
		}

		brush->Release();
#endif
	}

	void Graphics::DrawGradientFill(const Rectangle& rect, const Color& startColor, const Color& endColor)
	{
#ifdef BT_PLATFORM_WINDOWS
		
#endif
	}

	void Graphics::DrawButton(const Rectangle& rect, const Color& startColor, const Color& endColor, const Color& borderColor)
	{
#ifdef BT_PLATFORM_WINDOWS
		
#endif
	}

	void Graphics::DrawRoundedRectWithShadow(const Rectangle& rect, int radius, int shadowSize)
	{
		
	}

	void Graphics::DrawCircle(const Point& dest, int radius, const Color& fillColor, const Color& borderColor, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		
#endif
	}

	void Graphics::DrawEllipse(const Rectangle& dest, const Color& fillColor, const Color& borderColor, bool solid)
	{
#ifdef BT_PLATFORM_WINDOWS
		
#endif
	}

	void Graphics::Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destinationHandle, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::RootBufferNativeHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destinationHandle, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::NativeWindowHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS

#endif
	}

	void Graphics::Paste(API::RootBufferNativeHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		ID2D1Bitmap* sourceBitmap = nullptr;

		if (SUCCEEDED(m_attributes->m_bitmapRT->GetBitmap(&sourceBitmap)))
		{
			destinationHandle.m_renderTarget->BeginDraw();
			destinationHandle.m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

			destinationHandle.m_renderTarget->DrawBitmap
			(
				sourceBitmap,
				D2D1::RectF(static_cast<FLOAT>(dx), static_cast<FLOAT>(dy), static_cast<FLOAT>(dx + width), static_cast<FLOAT>(dy + height)),
				1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				D2D1::RectF(static_cast<FLOAT>(sx), static_cast<FLOAT>(sy), static_cast<FLOAT>(sx + width), static_cast<FLOAT>(sy + height))
			);

			auto hr = destinationHandle.m_renderTarget->EndDraw();
			if (FAILED(hr))
			{
				BT_CORE_ERROR << "error paste EndDraw()" << std::endl;
			}
		}
#endif
	}

	void Graphics::Begin()
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}
		
		m_attributes->m_bitmapRT->BeginDraw();
		m_attributes->m_bitmapRT->SetTransform(D2D1::Matrix3x2F::Identity());
#endif
	}

	void Graphics::Flush()
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		auto hr = m_attributes->m_bitmapRT->EndDraw();
		if (FAILED(hr))
		{
			_com_error err(hr);
			BT_CORE_ERROR << "Error Bitmap EndDraw(). err.ErrorMessage() = " << StringUtils::Convert(err.ErrorMessage()) << std::endl;
		}
#endif
	}

	void Graphics::Swap(Graphics& other)
	{
		std::swap(m_size, other.m_size);
		std::swap(m_nativeWindowHandle, other.m_nativeWindowHandle);
		std::swap(m_dpi, other.m_dpi);

		std::swap(m_attributes, other.m_attributes);
	}

	Size Graphics::GetTextExtent(const std::wstring& wstr)
	{
		return API::GetTextExtentSize(m_attributes.get(), wstr);
	}

	Size Graphics::GetTextExtent(const std::string& str)
	{
		return API::GetTextExtentSize(m_attributes.get(), str);
	}

	Size Graphics::GetTextExtent(const std::wstring& wstr, size_t length)
	{
		return API::GetTextExtentSize(m_attributes.get(), wstr, length);
	}

	void Graphics::Release()
	{
		m_attributes.reset();
				
		m_size = Size::Zero;
	}
}