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
#include "Berta/Platform/Windows/D2D.h"
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

	Graphics::Graphics(const Size& size, uint32_t dpi, API::RootPaintNativeHandle rootPaintHandle) :
		m_attributes(new PaintNativeHandle()),
		m_dpi(dpi)
	{
		Build(size, rootPaintHandle);
	}

	/*Graphics::Graphics(const Graphics& other)
	{
	}*/

	Graphics::Graphics(Graphics&& other) noexcept :
		m_attributes(std::move(other.m_attributes)),
		m_dpi(other.m_dpi),
		m_size(other.m_size),
		m_rootPaintNativeHandle(other.m_rootPaintNativeHandle)
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

	void Graphics::Build(const Size& size, API::RootPaintNativeHandle rootPaintHandle)
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

		m_rootPaintNativeHandle = rootPaintHandle;
#ifdef BT_PLATFORM_WINDOWS

		if (m_attributes->m_bitmapRT == nullptr)
		{
			D2D1_SIZE_F desiredSize = D2D1::SizeF(static_cast<FLOAT>(m_size.Width), static_cast<FLOAT>(m_size.Height));

			D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat
			(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_PREMULTIPLIED
			);

			auto hr = m_rootPaintNativeHandle.RenderTarget->CreateCompatibleRenderTarget
			(
				desiredSize, 
				D2D1::SizeU(m_size.Width, m_size.Height), 
				pixelFormat,
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

		if (FAILED(hr))
		{
			BT_CORE_ERROR << "Error building Font." << std::endl;
			return;
		}
		m_attributes->m_textExtent = GetTextExtent("{}[]");
#endif
	}

	void Graphics::Rebuild(const Size& size, API::RootPaintNativeHandle rootPaintHandle)
	{
		if (m_size == size && m_rootPaintNativeHandle == rootPaintHandle)
		{
			return;
		}

		Release();
		Build(size, rootPaintHandle);
	}

	void Graphics::Blend(const Rectangle& blendDestRectangle, const Graphics& graphicsSource, const Point& pointSource, double alpha)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		Rectangle sourceRect;
		sourceRect.X = pointSource.X;
		sourceRect.Y = pointSource.Y;
		sourceRect.Width = blendDestRectangle.Width;
		sourceRect.Height = blendDestRectangle.Height;

		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionClipRect(sourceRect, graphicsSource.GetSize(), blendDestRectangle, GetSize(), validSourceDest, validDestRect))
			return;

		ID2D1Bitmap* sourceBitmap = nullptr;
		if (SUCCEEDED(graphicsSource.m_attributes->m_bitmapRT->GetBitmap(&sourceBitmap)))
		{
			m_attributes->m_bitmapRT->DrawBitmap(sourceBitmap, validDestRect, static_cast<float>(alpha), D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, validSourceDest);

			sourceBitmap->Release();
		}
#endif
	}

	void Graphics::BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!graphicsSource.m_attributes || !graphicsSource.m_attributes->m_bitmapRT)
		{
			return;
		}

		ID2D1Bitmap* sourceBitmap = nullptr;
		if (SUCCEEDED(graphicsSource.m_attributes->m_bitmapRT->GetBitmap(&sourceBitmap)))
		{
			D2D1_RECT_F destRect = rectDestination;
			D2D1_RECT_F srcRect = D2D1::RectF(static_cast<FLOAT>(pointSource.X), static_cast<FLOAT>(pointSource.Y), static_cast<FLOAT>(pointSource.X + rectDestination.Width), static_cast<FLOAT>(pointSource.Y + rectDestination.Height));
			
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
		DrawLine(point1, point2, 1.0f, color, style);
	}

	void Graphics::DrawLine(const Point& point1, const Point& point2, float strokeWidth, const Color& color, LineStyle style)
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
				m_attributes->m_bitmapRT->DrawLine(point1F, point2F, brush, strokeWidth);
			}
			else
			{
				ID2D1StrokeStyle* strokeStyle = nullptr;

				D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties
				(
					D2D1_CAP_STYLE_ROUND,      // startCap
					D2D1_CAP_STYLE_ROUND,      // endCap
					D2D1_CAP_STYLE_FLAT,       // dashCap
					D2D1_LINE_JOIN_ROUND,      // lineJoin
					10.0f,                     // miterLimit
					D2D1_DASH_STYLE_DASH,      // dashStyle
					0.0f                       // dashOffset
				);

				DirectX::D2DModule::GetInstance().GetFactory()->CreateStrokeStyle(&props, nullptr, 0, &strokeStyle);
				m_attributes->m_bitmapRT->DrawLine(point1F, point2F, brush, strokeWidth, strokeStyle);

				strokeStyle->Release();
			}
			brush->Release();
		}
#endif
	}

	void Graphics::DrawBeginLine(const Point& point, const Color& color, LineStyle style)
	{
#ifdef BT_PLATFORM_WINDOWS
#endif
	}

	void Graphics::DrawLineTo(const Point& point, const Color& color)
	{
#ifdef BT_PLATFORM_WINDOWS
#endif
	}

	void Graphics::DrawRectangle(const Color& color, bool solid, float strokeWidth)
	{
		DrawRectangle(m_size.ToRectangle(), color, solid, strokeWidth);
	}

	void Graphics::DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		Rectangle validRectangle;
		if (!LayoutUtils::GetIntersectionClipRect(GetSize().ToRectangle(), rectangle, validRectangle))
		{
			return;
		}

		D2D1_RECT_F d2dRect = validRectangle;

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
				d2dRect.left += 0.5f;
				d2dRect.top += 0.5f;
				d2dRect.right -= 0.5f;
				d2dRect.bottom -= 0.5f;

				m_attributes->m_bitmapRT->DrawRectangle(&d2dRect, brush, strokeWidth);
			}
			brush->Release();
		}
#endif
	}

	void Graphics::DrawRectangle(const Rectangle& rectangle, const Color& borderColor, bool solid, const Color& solidColor, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		Rectangle validRectangle;
		if (!LayoutUtils::GetIntersectionClipRect(GetSize().ToRectangle(), rectangle, validRectangle))
		{
			return;
		}

		D2D1_RECT_F d2dRect = validRectangle;
		
		ID2D1SolidColorBrush* borderBrush;
		auto hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(borderColor, &borderBrush);

		if (SUCCEEDED(hr))
		{
			if (solid)
			{
				ID2D1SolidColorBrush* solidBrush;
				hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(solidColor, &solidBrush);
				if (SUCCEEDED(hr))
				{
					m_attributes->m_bitmapRT->FillRectangle(&d2dRect, solidBrush);
					solidBrush->Release();
				}
			}
			
			d2dRect.left += 0.5f;
			d2dRect.top += 0.5f;
			d2dRect.right -= 0.5f;
			d2dRect.bottom -= 0.5f;

			m_attributes->m_bitmapRT->DrawRectangle(&d2dRect, borderBrush);

			borderBrush->Release();
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
		d2dRect.right = static_cast<FLOAT>(position.X + textSize.Width);
		d2dRect.bottom = static_cast<FLOAT>(position.Y + textSize.Height);

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

	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor, bool solid, const Color& solidColor, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		Rectangle output;
		if (!LayoutUtils::GetIntersectionClipRect(GetSize().ToRectangle(), rect, output))
		{
			return;
		}

		D2D1_POINT_2F p1, p2, p3;
		Point center{};
		center.X = (rect.X * 2 + rect.Width) >> 1;
		center.Y = (rect.Y * 2 + rect.Height) >> 1;

		switch (direction)
		{
		case ArrowDirection::Upwards:
			p1 = D2D1::Point2F(static_cast<float>(center.X), static_cast<float>(center.Y - arrowLength));
			p2 = D2D1::Point2F(static_cast<float>(center.X - arrowWidth), static_cast<float>(center.Y + arrowLength));
			p3 = D2D1::Point2F(static_cast<float>(center.X + arrowWidth), static_cast<float>(center.Y + arrowLength));
			break;

		case ArrowDirection::Downwards:
			p1 = D2D1::Point2F(static_cast<float>(center.X), static_cast<float>(center.Y + arrowLength));
			p2 = D2D1::Point2F(static_cast<float>(center.X - arrowWidth), static_cast<float>(center.Y - arrowLength));
			p3 = D2D1::Point2F(static_cast<float>(center.X + arrowWidth), static_cast<float>(center.Y - arrowLength));
			break;

		case ArrowDirection::Left:
			p1 = D2D1::Point2F(static_cast<float>(center.X - arrowLength), static_cast<float>(center.Y));
			p2 = D2D1::Point2F(static_cast<float>(center.X + arrowLength), static_cast<float>(center.Y - arrowWidth));
			p3 = D2D1::Point2F(static_cast<float>(center.X + arrowLength), static_cast<float>(center.Y + arrowWidth));
			break;

		case ArrowDirection::Right:
			p1 = D2D1::Point2F(static_cast<float>(center.X + arrowLength), static_cast<float>(center.Y));
			p2 = D2D1::Point2F(static_cast<float>(center.X - arrowLength), static_cast<float>(center.Y - arrowWidth));
			p3 = D2D1::Point2F(static_cast<float>(center.X - arrowLength), static_cast<float>(center.Y + arrowWidth));
			break;
		}

		if (!output.IsInside(p1) && !output.IsInside(p2) && !output.IsInside(p3))
			return;

		ID2D1PathGeometry* geometry = nullptr;
		ID2D1GeometrySink* sink = nullptr;

		auto hr = DirectX::D2DModule::GetInstance().GetFactory()->CreatePathGeometry(&geometry);
		if (FAILED(hr))
		{
			return;
		}

		hr = geometry->Open(&sink);
		if (FAILED(hr))
		{
			geometry->Release();
			return;
		}

		sink->BeginFigure(p1, solid ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
		sink->AddLine(p2);
		sink->AddLine(p3);
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		sink->Close();

		ID2D1SolidColorBrush* borderBrush;
		hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(borderColor, &borderBrush);

		if (SUCCEEDED(hr))
		{
			if (solid)
			{
				ID2D1SolidColorBrush* solidBrush;
				hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(solidColor, &solidBrush);
				if (SUCCEEDED(hr))
				{
					m_attributes->m_bitmapRT->FillGeometry(geometry, solidBrush);
					solidBrush->Release();
				}
			}

			m_attributes->m_bitmapRT->DrawGeometry(geometry, borderBrush, strokeWidth);

			borderBrush->Release();

		}
		sink->Release();
		geometry->Release();
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

		Rectangle output;
		if (!LayoutUtils::GetIntersectionClipRect(GetSize().ToRectangle(), rect, output))
		{
			return;
		}

		float scaleFactor = LayoutUtils::CalculateDPIScaleFactor(m_dpi);
		auto radiusScaled = radius * scaleFactor;
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect
		(
			D2D1::RectF(static_cast<FLOAT>(rect.X) + 0.5f, static_cast<FLOAT>(rect.Y + 0.5f),
				static_cast<FLOAT>(rect.X + rect.Width) - 0.5f, static_cast<FLOAT>(rect.Y + rect.Height) - 0.5f),
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
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		D2D1_GRADIENT_STOP gradientStops[2];
		gradientStops[0].position = 0.0f;
		gradientStops[0].color = startColor;

		gradientStops[1].position = 1.0f;
		gradientStops[1].color = endColor;

		ID2D1GradientStopCollection* pGradientStopCollection = nullptr;
		auto hr = m_attributes->m_bitmapRT->CreateGradientStopCollection(
			gradientStops,
			2,
			D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			&pGradientStopCollection
		);

		if (FAILED(hr))
		{
			return;
		}

		ID2D1LinearGradientBrush* pLinearGradientBrush = nullptr;
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES linearGradientBrushProperties =
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(0.0f, 0.0f),
				D2D1::Point2F(0.0f, static_cast<float>(rect.Height))
			);

		hr = m_attributes->m_bitmapRT->CreateLinearGradientBrush(
			linearGradientBrushProperties,
			pGradientStopCollection,
			&pLinearGradientBrush
		);

		if (FAILED(hr))
		{
			return;
		}

		D2D1_RECT_F d2dRect = rect;
		m_attributes->m_bitmapRT->FillRectangle(&d2dRect, pLinearGradientBrush);

		if (pLinearGradientBrush) pLinearGradientBrush->Release();
		if (pGradientStopCollection) pGradientStopCollection->Release();
#endif
	}

	void Graphics::DrawCircle(const Point& dest, int radius, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(dest.X, dest.Y), static_cast<float>(radius), static_cast<float>(radius));

		ID2D1SolidColorBrush* borderBrush;
		auto hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(borderColor, &borderBrush);
		if (FAILED(hr))
		{
			return;
		}

		if (solid)
		{
			ID2D1SolidColorBrush* fillBrush;
			hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(fillColor, &fillBrush);
			if (FAILED(hr))
			{
				borderBrush->Release();
				return;
			}

			m_attributes->m_bitmapRT->FillEllipse(ellipse, fillBrush);
			fillBrush->Release();
		}

		m_attributes->m_bitmapRT->DrawEllipse(ellipse, borderBrush, strokeWidth);

		borderBrush->Release();
#endif
	}

	void Graphics::DrawEllipse(const Rectangle& dest, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(static_cast<FLOAT>((dest.X * 2 + dest.Width)>> 1), static_cast<FLOAT>((dest.Y * 2 + dest.Height) >> 1)), 
			static_cast<FLOAT>(dest.Width >> 1), static_cast<FLOAT>(dest.Height >> 1));

		ID2D1SolidColorBrush* borderBrush;
		auto hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(borderColor, &borderBrush);
		if (FAILED(hr))
		{
			return;
		}

		if (solid)
		{
			ID2D1SolidColorBrush* fillBrush;
			hr = m_attributes->m_bitmapRT->CreateSolidColorBrush(fillColor, &fillBrush);
			if (FAILED(hr))
			{
				borderBrush->Release();
				return;
			}

			m_attributes->m_bitmapRT->FillEllipse(ellipse, fillBrush);
			fillBrush->Release();
		}

		m_attributes->m_bitmapRT->DrawEllipse(ellipse, borderBrush, strokeWidth);

		borderBrush->Release();
#endif
	}

	void Graphics::Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destinationHandle, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::RootPaintNativeHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const
	{
		Paste(destinationHandle, areaToUpdate.X, areaToUpdate.Y, areaToUpdate.Width, areaToUpdate.Height, x, y);
	}

	void Graphics::Paste(API::NativeWindowHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS

#endif
	}

	void Graphics::Paste(API::RootPaintNativeHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_attributes->m_bitmapRT)
		{
			return;
		}

		ID2D1Bitmap* sourceBitmap = nullptr;
		if (SUCCEEDED(m_attributes->m_bitmapRT->GetBitmap(&sourceBitmap)))
		{
			auto destRect = D2D1::RectF(static_cast<FLOAT>(dx), static_cast<FLOAT>(dy), static_cast<FLOAT>(dx + width), static_cast<FLOAT>(dy + height));
			auto sourceRect = D2D1::RectF(static_cast<FLOAT>(sx), static_cast<FLOAT>(sy), static_cast<FLOAT>(sx + width), static_cast<FLOAT>(sy + height));
			
			destinationHandle.RenderTarget->BeginDraw();
			destinationHandle.RenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			destinationHandle.RenderTarget->DrawBitmap
			(
				sourceBitmap,
				destRect,
				1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				sourceRect
			);

			auto hr = destinationHandle.RenderTarget->EndDraw();
			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Error on Paste method, EndDraw()" << std::endl;
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
		std::swap(m_rootPaintNativeHandle, other.m_rootPaintNativeHandle);
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

	bool Graphics::IsEnabledAliasing()
	{
#ifdef BT_PLATFORM_WINDOWS
		return m_attributes->m_bitmapRT->GetAntialiasMode() == D2D1_ANTIALIAS_MODE_ALIASED;
#else
		return false;
#endif
	}

	void Graphics::EnabledAliasing(bool enabled)
	{
#ifdef BT_PLATFORM_WINDOWS
		m_attributes->m_bitmapRT->SetAntialiasMode(enabled ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#endif
	}
}
