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

#ifdef BT_DEBUG
#define BT_GRAPHICS_DEBUG_ERROR_MESSAGES
#endif

namespace Berta
{
	Graphics::Graphics() :
		m_attributes(std::make_unique<PaintNativeHandle>())
	{
	}

	Graphics::Graphics(const Size& size, uint32_t dpi, API::RootPaintNativeHandle rootPaintHandle) :
		m_attributes(std::make_unique<PaintNativeHandle>()),
		m_dpi(dpi)
	{
		Build(size, rootPaintHandle);
	}

	Graphics::Graphics(API::RootPaintNativeHandle rootPaintHandle) :
		m_attributes(std::make_unique<PaintNativeHandle>())
	{
		Build(rootPaintHandle);
	}

	Graphics::Graphics(Graphics&& other) noexcept :
		m_attributes(std::move(other.m_attributes)),
		m_dpi(other.m_dpi),
		m_size(other.m_size),
		m_rootPaintNativeHandle(other.m_rootPaintNativeHandle),
#ifdef BT_PLATFORM_WINDOWS
		m_resourceCache(std::move(other.m_resourceCache)),
#endif
		m_targetRT(other.m_targetRT)
	{
		other.m_attributes = std::make_unique<PaintNativeHandle>();
		other.m_targetRT = nullptr;
	}

	Graphics::~Graphics()
	{
		Release();
	}

	Graphics& Graphics::operator=(Graphics&& other) noexcept
	{
		if (this != &other)
		{
			m_attributes = std::move(other.m_attributes);
			m_dpi = other.m_dpi;
			m_size = other.m_size;
#ifdef BT_PLATFORM_WINDOWS
			m_resourceCache = std::move(other.m_resourceCache);
#endif
			m_targetRT = other.m_targetRT;
			
			other.m_targetRT = nullptr;
		}

		return *this;
	}

	void Graphics::Build(API::RootPaintNativeHandle rootPaintHandle)
	{
		if (!m_attributes)
		{
			m_attributes = std::make_unique<PaintNativeHandle>();
		}

		m_rootPaintNativeHandle = rootPaintHandle;

#ifdef BT_PLATFORM_WINDOWS
		if (!rootPaintHandle.RenderTarget)
		{
			return;
		}

		m_targetRT = rootPaintHandle.RenderTarget;

		auto [width, height] = rootPaintHandle.RenderTarget->GetSize();
		m_size.Width = static_cast<uint32_t>(width);
		m_size.Height = static_cast<uint32_t>(height);

		m_resourceCache = ResourceCache(m_targetRT);
#endif
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
			m_attributes = std::make_unique<PaintNativeHandle>();
		}

		m_rootPaintNativeHandle = rootPaintHandle;
		
#ifdef BT_PLATFORM_WINDOWS
		if (m_attributes->m_bitmapRT == nullptr && m_rootPaintNativeHandle.RenderTarget)
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

			m_targetRT = m_attributes->m_bitmapRT;
		}
		m_resourceCache = ResourceCache(m_targetRT);
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

		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextFormat
		(
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

	void Graphics::Rebuild(API::RootPaintNativeHandle rootPaintHandle)
	{
		if (m_rootPaintNativeHandle == rootPaintHandle)
		{
			return;
		}

		Release();
		Build(rootPaintHandle);
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

	void Graphics::CreateTextLayout(const wchar_t* wstr, UINT32 length, uint32_t width, uint32_t height)
	{
#ifdef BT_PLATFORM_WINDOWS
		Microsoft::WRL::ComPtr<IDWriteTextLayout> tempLayout;
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			wstr,
			length,
			m_attributes->m_textFormat,
			static_cast<float>(width),
			static_cast<float>(height),
			&tempLayout
		);
		
		if (SUCCEEDED(hr))
		{
		}
#endif
	}

	void Graphics::Blend(const Rectangle& blendDestRectangle, const Graphics& graphicsSource, const Point& pointSource, double alpha)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!IsValid() || !graphicsSource.IsValid())
		{
			return;
		}

		Rectangle sourceRect;
		sourceRect.X = pointSource.X;
		sourceRect.Y = pointSource.Y;
		sourceRect.Width = blendDestRectangle.Width;
		sourceRect.Height = blendDestRectangle.Height;

		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionRect(sourceRect, graphicsSource.GetSize(), blendDestRectangle, GetSize(), validSourceDest, validDestRect))
		{
			return;
		}

		ID2D1Bitmap* sourceBitmap = nullptr;
		if (SUCCEEDED(graphicsSource.m_attributes->m_bitmapRT->GetBitmap(&sourceBitmap)))
		{
			m_targetRT->DrawBitmap(sourceBitmap, validDestRect, static_cast<float>(alpha), D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, validSourceDest);

			sourceBitmap->Release();
		}
#endif
	}

	void Graphics::BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!IsValid() || !graphicsSource.IsValid())
		{
			return;
		}

		ID2D1Bitmap* sourceBitmap = nullptr;
		if (SUCCEEDED(graphicsSource.m_attributes->m_bitmapRT->GetBitmap(&sourceBitmap)))
		{
			D2D1_RECT_F destRect = rectDestination;
			D2D1_RECT_F srcRect = D2D1::RectF(static_cast<FLOAT>(pointSource.X), static_cast<FLOAT>(pointSource.Y), static_cast<FLOAT>(pointSource.X + rectDestination.Width), static_cast<FLOAT>(pointSource.Y + rectDestination.Height));
			
			m_targetRT->DrawBitmap(sourceBitmap, destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, srcRect);
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
		if (!IsValid())
		{
			return;
		}

		auto brush = m_resourceCache.GetBrush(color);
		if (brush)
		{
			D2D1_POINT_2F point1F;
			point1F.x = static_cast<FLOAT>(point1.X) + 0.5f;
			point1F.y = static_cast<FLOAT>(point1.Y) + 0.5f;

			D2D1_POINT_2F point2F;
			point2F.x = static_cast<FLOAT>(point2.X) + 0.5f;
			point2F.y = static_cast<FLOAT>(point2.Y) + 0.5f;

			if (style == LineStyle::Solid)
			{
				m_targetRT->DrawLine(point1F, point2F, brush, strokeWidth);
			}
			else
			{
				float dashes[] = { 1.0f, 1.0f };
				ID2D1StrokeStyle* strokeStyle = nullptr;

				/*D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties
				(
					D2D1_CAP_STYLE_ROUND,      // startCap
					D2D1_CAP_STYLE_ROUND,      // endCap
					D2D1_CAP_STYLE_FLAT,       // dashCap
					D2D1_LINE_JOIN_ROUND,      // lineJoin
					10.0f,                     // miterLimit
					D2D1_DASH_STYLE_DASH,      // dashStyle
					0.0f                       // dashOffset
				);*/
				
				D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties(
					D2D1_CAP_STYLE_FLAT,  // Cap inicial plano
					D2D1_CAP_STYLE_FLAT,  // Cap final plano
					D2D1_CAP_STYLE_FLAT,  // Cap intermedio plano
					D2D1_LINE_JOIN_MITER, // Unión recta
					10.0f,                // Límite de miter
					D2D1_DASH_STYLE_CUSTOM, // Usaremos nuestro array de arriba
					0.0f                  // Desplazamiento inicial del patrón
				);

				DirectX::D2DModule::GetInstance().GetFactory()->CreateStrokeStyle(&props, dashes, ARRAYSIZE(dashes), &strokeStyle);
				m_targetRT->DrawLine(point1F, point2F, brush, strokeWidth, strokeStyle);

				strokeStyle->Release();
			}
		}
#endif
	}

	void Graphics::DrawRectangle(const Rectangle& rect, const Color& borderColor, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!IsValid())
		{
			return;
		}

		Rectangle validRectangle;
		if (!LayoutUtils::GetIntersectionRect(GetSize().ToRectangle(), rect, validRectangle))
		{
			return;
		}
		
		auto borderBrush = m_resourceCache.GetBrush(borderColor);
		if (borderBrush)
		{
			D2D1_RECT_F d2dRect = validRectangle;
			
			d2dRect.left += 0.5f;
			d2dRect.top += 0.5f;
			d2dRect.right -= 0.5f;
			d2dRect.bottom -= 0.5f;

			m_targetRT->DrawRectangle(&d2dRect, borderBrush, strokeWidth);
		}
#endif
	}

	void Graphics::FillRectangle(const Rectangle& rect, const Color& fillColor)
	{
		if (!IsValid())
		{
			return;
		}
		
#ifdef BT_PLATFORM_WINDOWS
		Rectangle validRectangle;
		if (!LayoutUtils::GetIntersectionRect(GetSize().ToRectangle(), rect, validRectangle))
		{
			return;
		}

		D2D1_RECT_F d2dRect = validRectangle;
		
		auto fillBrush = m_resourceCache.GetBrush(fillColor);
		if (fillBrush)
		{
			m_targetRT->FillRectangle(&d2dRect, fillBrush);
		}
#endif
	}

	void Graphics::FillAndDrawRectangle(const Rectangle& rect, const Color& solidColor, const Color& borderColor, float strokeWidth)
	{
		if (!IsValid())
		{
			return;
		}
		
#ifdef BT_PLATFORM_WINDOWS

		Rectangle validRectangle;
		if (!LayoutUtils::GetIntersectionRect(GetSize().ToRectangle(), rect, validRectangle))
		{
			return;
		}

		D2D1_RECT_F d2dRect = validRectangle;
		
		auto borderBrush = m_resourceCache.GetBrush(borderColor);
		auto solidBrush = m_resourceCache.GetBrush(solidColor);
		if (borderBrush)
		{
			if (solidBrush)
			{
				m_targetRT->FillRectangle(&d2dRect, solidBrush);
			}
			
			d2dRect.left += 0.5f;
			d2dRect.top += 0.5f;
			d2dRect.right -= 0.5f;
			d2dRect.bottom -= 0.5f;

			m_targetRT->DrawRectangle(&d2dRect, borderBrush, strokeWidth);
		}
#endif
	}
	
	void Graphics::DrawTopRoundedRectangle(const Rectangle& rect, float radius, const Color& borderColor, bool closeFigure, float strokeWidth)
	{
		if (!IsValid())
		{
			return;
		}
		
		if (radius <= 0.0f)
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		if (auto pathGeometry = CreateTopRoundedGeometry(rect, radius, closeFigure))
		{
			if (auto brush = m_resourceCache.GetBrush(borderColor))
			{
				m_targetRT->DrawGeometry(pathGeometry, brush, strokeWidth);
			}
			pathGeometry->Release();
		}
		
		/*ID2D1PathGeometry* pathGeometry = nullptr;
		HRESULT hr = DirectX::D2DModule::GetInstance().GetFactory()->CreatePathGeometry(&pathGeometry);
		
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink* sink = nullptr;
			hr = pathGeometry->Open(&sink);
			
			if (SUCCEEDED(hr))
			{
				// ¡LA MAGIA DEL 0.5f AQUÍ!
				// Desplazamos las coordenadas para alinear el trazo a la cuadrícula de píxeles
				//float offset = drawBorder ? 0.5f : 0.0f; 
				float offset = 0.5f; 
				float left = static_cast<float>(rect.X) + offset;
				float top = static_cast<float>(rect.Y) + offset;
				float right = static_cast<float>(rect.X + rect.Width) - offset;
				float bottom = static_cast<float>(rect.Y + rect.Height) - offset;

				// 1. Empezamos en la esquina inferior izquierda
				sink->BeginFigure(D2D1::Point2F(left, bottom), D2D1_FIGURE_BEGIN_FILLED);

				// 2. Línea izquierda hacia arriba
				sink->AddLine(D2D1::Point2F(left, top + radius));

				// 3. Arco superior izquierdo
				sink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(left + radius, top),
					D2D1::SizeF(radius, radius),
					0.0f, 
					D2D1_SWEEP_DIRECTION_CLOCKWISE, 
					D2D1_ARC_SIZE_SMALL
				));

				// 4. Línea superior
				sink->AddLine(D2D1::Point2F(right - radius, top));

				// 5. Arco superior derecho
				sink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(right, top + radius),
					D2D1::SizeF(radius, radius), 
					0.0f, 
					D2D1_SWEEP_DIRECTION_CLOCKWISE, 
					D2D1_ARC_SIZE_SMALL
				));

				// 6. Línea derecha hacia abajo
				sink->AddLine(D2D1::Point2F(right, bottom));

				// ¡LA MAGIA DE LA LÍNEA INFERIOR AQUÍ!
				// Si open, no dibuja la línea que conecta el punto derecho con el izquierdo.
				sink->EndFigure(closeFigure ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
				sink->Close();
				
				auto pBorderBrush = m_resourceCache.GetBrush(borderColor);
				m_targetRT->DrawGeometry(pathGeometry, pBorderBrush, 1.0f);
				
				// Liberación de recursos locales
				sink->Release();
			}
			pathGeometry->Release();
		}*/
#endif
	}

	void Graphics::FillTopRoundedRectangle(const Rectangle& rect, float radius, const Color& fillColor)
	{
		if (radius <= 0.0f)
		{
			FillRectangle(rect, fillColor);
			return;
		}
		if (!IsValid())
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		if (auto pathGeometry = CreateTopRoundedGeometry(rect, radius, false))
		{
			if (auto brush = m_resourceCache.GetBrush(fillColor))
			{
				m_targetRT->FillGeometry(pathGeometry, brush);
			}
			pathGeometry->Release();
		}
#endif
	}

	void Graphics::DrawBottomRoundedRectangle(const Rectangle& rect, float radius, Color fillColor, Color borderColor, bool closeFigure, float strokeWidth)
	{
		if (!IsValid())
		{
			return;
		}
		
		if (radius <= 0.0f)
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		if (auto pathGeometry = CreateBottomRoundedGeometry(rect, radius, closeFigure))
		{
			if (auto brush = m_resourceCache.GetBrush(borderColor))
			{
				m_targetRT->DrawGeometry(pathGeometry, brush, strokeWidth);
			}
			pathGeometry->Release();
		}
#endif
		/*if (radius <= 0.0f)
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		ID2D1PathGeometry* pathGeometry = nullptr;
		HRESULT hr = DirectX::D2DModule::GetInstance().GetFactory()->CreatePathGeometry(&pathGeometry);
	
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink* sink = nullptr;
			hr = pathGeometry->Open(&sink);
		
			if (SUCCEEDED(hr))
			{
				float left = static_cast<float>(rect.X);
				float top = static_cast<float>(rect.Y);
				float right = left + static_cast<float>(rect.Width);
				float bottom = top + static_cast<float>(rect.Height);

				// 1. Empezamos en la esquina superior izquierda (Cuadrada)
				sink->BeginFigure(D2D1::Point2F(left, top), D2D1_FIGURE_BEGIN_FILLED);

				// 2. Línea recta hacia la esquina superior derecha (Cuadrada)
				sink->AddLine(D2D1::Point2F(right, top));

				// 3. Línea recta hacia abajo, hasta justo antes de la curva inferior derecha
				sink->AddLine(D2D1::Point2F(right, bottom - radius));

				// 4. Arco para la esquina inferior derecha
				sink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(right - radius, bottom), // Punto de destino: nos movemos a la izquierda
					D2D1::SizeF(radius, radius),
					0.0f, 
					D2D1_SWEEP_DIRECTION_CLOCKWISE, 
					D2D1_ARC_SIZE_SMALL
				));

				// 5. Línea recta hacia la izquierda, hasta justo antes de la curva inferior izquierda
				sink->AddLine(D2D1::Point2F(left + radius, bottom));

				// 6. Arco para la esquina inferior izquierda
				sink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(left, bottom - radius), // Punto de destino: subimos por el borde izquierdo
					D2D1::SizeF(radius, radius), 
					0.0f, 
					D2D1_SWEEP_DIRECTION_CLOCKWISE, 
					D2D1_ARC_SIZE_SMALL
				));

				// 7. Cerramos la figura (dibuja la línea izquierda hacia arriba conectando con el inicio)
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				sink->Close();

				auto pFillBrush = m_resourceCache.GetBrush(fillColor);
				m_targetRT->FillGeometry(pathGeometry, pFillBrush);

				auto pBorderBrush = m_resourceCache.GetBrush(borderColor);
				m_targetRT->DrawGeometry(pathGeometry, pBorderBrush, 1.0f); // 1.0f es el grosor de la línea

				// Liberación de recursos locales
				sink->Release();
			}
			pathGeometry->Release();
		}
#endif*/
	}

	void Graphics::FillBottomRoundedRectangle(const Rectangle& rect, float radius, Color fillColor)
	{
	}

	void Graphics::DrawString(const Point& position, std::string_view strView, const Color& color) 
	{
		DrawString(position, StringUtils::UTF8ToWide(std::string(strView)), color);
	}

	void Graphics::DrawString(const Point& position, std::wstring_view wstrView, const Color& color)
	{
		if (wstrView.empty() || !IsValid())
		{
			return;
		}

#ifdef BT_PLATFORM_WINDOWS
		auto textSize = GetTextExtent(wstrView);
		auto wstr = std::wstring{ wstrView };

		D2D1_RECT_F d2dRect;
		d2dRect.left = static_cast<FLOAT>(position.X);
		d2dRect.top = static_cast<FLOAT>(position.Y);
		d2dRect.right = static_cast<FLOAT>(position.X + textSize.Width);
		d2dRect.bottom = static_cast<FLOAT>(position.Y + textSize.Height);

		auto brush = m_resourceCache.GetBrush(color);
		
		IDWriteTextLayout* textLayout = nullptr;
		
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			wstr.data(), 
			static_cast<UINT32>(wstr.size()),
			m_attributes->m_textFormat,
			FLT_MAX, FLT_MAX,
			&textLayout
		);
		
		if (FAILED(hr))
		{
			return;
		}

		m_targetRT->DrawText
		(
			wstr.data(),
			static_cast<UINT32>(wstr.size()),
			m_attributes->m_textFormat,
			d2dRect,
			brush
		);
		
		textLayout->Release();
#endif
	}

	void Graphics::DrawString(const Rectangle& area, std::string_view strView, const Color& color, const TextFormatOptions& options)
	{
		DrawString(area, StringUtils::UTF8ToWide(std::string(strView)), color, options);
	}

	void Graphics::DrawString(const Rectangle& area, std::wstring_view wstrView, const Color& color, const TextFormatOptions& options)
	{
		if (wstrView.empty() || !IsValid())
		{
			return;
		}
		
#ifdef BT_PLATFORM_WINDOWS
		auto wstr = std::wstring(wstrView);
		IDWriteTextLayout* textLayout = nullptr;
		
		HRESULT hr = DirectX::D2DModule::GetInstance().GetWriteFactory()->CreateTextLayout
		(
			wstr.c_str(), 
			static_cast<UINT32>(wstr.size()),
			m_attributes->m_textFormat,
			static_cast<FLOAT>(area.Width), static_cast<FLOAT>(area.Height),
			&textLayout
		);

		if (FAILED(hr))
		{
			return;
		}

		auto brush = m_resourceCache.GetBrush(color);
		
		textLayout->SetWordWrapping(options.WordWrap ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
		textLayout->SetTextAlignment(options.AlignX == HorizontalAlign::Left ? DWRITE_TEXT_ALIGNMENT_LEADING : 
			(options.AlignX == HorizontalAlign::Center ? DWRITE_TEXT_ALIGNMENT_CENTER : DWRITE_TEXT_ALIGNMENT_TRAILING));
		
		textLayout->SetParagraphAlignment(options.AlignY == VerticalAlign::Top ? DWRITE_PARAGRAPH_ALIGNMENT_NEAR : 
			(options.AlignY == VerticalAlign::Center ? DWRITE_PARAGRAPH_ALIGNMENT_CENTER : DWRITE_PARAGRAPH_ALIGNMENT_FAR));
		
		D2D1_RECT_F d2dRect;
		d2dRect.left = static_cast<FLOAT>(area.X);
		d2dRect.top = static_cast<FLOAT>(area.Y);
		d2dRect.right = static_cast<FLOAT>(area.X + area.Width);
		d2dRect.bottom = static_cast<FLOAT>(area.Y + area.Height);
		
		m_targetRT->DrawText
		(
			wstr.c_str(),
			static_cast<UINT32>(wstr.size()),
			m_attributes->m_textFormat,
			d2dRect,
			brush,
			D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT | D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
		);
		/*m_targetRT->DrawTextLayout
		(
			D2D1_POINT_2F {static_cast<FLOAT>(area.X), static_cast<FLOAT>(area.Y)},
			textLayout,
			brush
		);*/
		
		textLayout->Release();
#endif
	}

	void Graphics::DrawTextLayout(const TextPaintNativeHandle& handle, const Point& origin, const Color& color)
	{
		if (!handle.IsValid())
		{
			return;
		}
		
#ifdef BT_PLATFORM_WINDOWS
		auto brush = m_resourceCache.GetBrush(color);
		if (brush)
		{
			m_targetRT->DrawTextLayout
			(
				D2D1::Point2F(static_cast<float>(origin.X), static_cast<float>(origin.Y)),
				handle.m_textLayout.Get(),
				brush,
				D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
			);
		}
#endif
	}

	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor)
	{
		DrawArrow(rect, arrowLength, arrowWidth, direction, borderColor, false, borderColor);
	}

	void Graphics::DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor, bool solid, const Color& solidColor, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		Rectangle output;
		if (!LayoutUtils::GetIntersectionRect(GetSize().ToRectangle(), rect, output))
		{
			return;
		}

		D2D1_POINT_2F p1, p2, p3;
		Point center;
		center.X = (rect.X * 2 + static_cast<int>(rect.Width)) >> 1;
		center.Y = (rect.Y * 2 + static_cast<int>(rect.Height)) >> 1;

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
		{
			return;
		}

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

		auto borderBrush = m_resourceCache.GetBrush(borderColor);
		if (borderBrush)
		{
			if (solid)
			{
				auto solidBrush = m_resourceCache.GetBrush(solidColor);
				if (solidBrush)
				{
					m_targetRT->FillGeometry(geometry, solidBrush);
				}
			}

			m_targetRT->DrawGeometry(geometry, borderBrush, strokeWidth);
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
		if (!IsValid())
		{
			return;
		}

		Rectangle output;
		if (!LayoutUtils::GetIntersectionRect(GetSize().ToRectangle(), rect, output))
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

		auto brush = m_resourceCache.GetBrush(color);
		if (!brush)
		{
			return;
		}

		auto brushBorder = m_resourceCache.GetBrush(bordercolor);
		if (brushBorder)
		{
			if (solid)
			{
				m_targetRT->FillRoundedRectangle(&roundedRect, brush);
				m_targetRT->DrawRoundedRectangle(&roundedRect, brushBorder);
			}
			else
			{

				m_targetRT->DrawRoundedRectangle(&roundedRect, brushBorder);
			}
		}
#endif
	}

	void Graphics::DrawGradientFill(const Rectangle& rect, const Color& startColor, const Color& endColor)
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!IsValid())
		{
			return;
		}

		D2D1_GRADIENT_STOP gradientStops[2];
		gradientStops[0].position = 0.0f;
		gradientStops[0].color = startColor;

		gradientStops[1].position = 1.0f;
		gradientStops[1].color = endColor;

		ID2D1GradientStopCollection* pGradientStopCollection = nullptr;
		auto hr = m_targetRT->CreateGradientStopCollection(
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

		hr = m_targetRT->CreateLinearGradientBrush(
			linearGradientBrushProperties,
			pGradientStopCollection,
			&pLinearGradientBrush
		);

		if (FAILED(hr))
		{
			return;
		}

		D2D1_RECT_F d2dRect = rect;
		m_targetRT->FillRectangle(&d2dRect, pLinearGradientBrush);

		if (pLinearGradientBrush) pLinearGradientBrush->Release();
		if (pGradientStopCollection) pGradientStopCollection->Release();
#endif
	}

	void Graphics::DrawCircle(const Point& dest, int radius, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(dest.X, dest.Y), static_cast<float>(radius), static_cast<float>(radius));

		auto borderBrush = m_resourceCache.GetBrush(borderColor);
		if (!borderBrush)
		{
			return;
		}

		if (solid)
		{
			auto fillBrush = m_resourceCache.GetBrush(fillColor);
			if (!fillBrush)
			{
				return;
			}

			m_targetRT->FillEllipse(ellipse, fillBrush);
		}

		m_targetRT->DrawEllipse(ellipse, borderBrush, strokeWidth);
#endif
	}

	void Graphics::DrawEllipse(const Rectangle& dest, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth)
	{
#ifdef BT_PLATFORM_WINDOWS
		D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(static_cast<FLOAT>((dest.X * 2 + dest.Width)>> 1), static_cast<FLOAT>((dest.Y * 2 + dest.Height) >> 1)), 
			static_cast<FLOAT>(dest.Width >> 1), static_cast<FLOAT>(dest.Height >> 1));

		auto borderBrush = m_resourceCache.GetBrush(borderColor);
		if (!borderBrush)
		{
			return;
		}

		if (solid)
		{
			auto fillBrush = m_resourceCache.GetBrush(fillColor);
			if (!fillBrush)
			{
				return;
			}

			m_targetRT->FillEllipse(ellipse, fillBrush);
		}

		m_targetRT->DrawEllipse(ellipse, borderBrush, strokeWidth);
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
		if (!IsValid())
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
				return;
			}

			sourceBitmap->Release();
		}
#endif
	}

	void Graphics::Begin()
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!IsValid())
		{
			return;
		}

		m_targetRT->BeginDraw();
#endif
	}

	void Graphics::Flush()
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!IsValid())
		{
			return;
		}

		auto hr = m_targetRT->EndDraw();
		if (FAILED(hr))
		{
			_com_error err(hr);
			BT_CORE_ERROR << "Error Bitmap EndDraw(). err.ErrorMessage() = " << StringUtils::WideToUTF8(err.ErrorMessage()) << std::endl;
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

	Size Graphics::GetTextExtent(const std::wstring& wstr) const
	{
		return API::GetTextExtentSize(m_attributes.get(), wstr);
	}

	Size Graphics::GetTextExtent(const std::string& str) const
	{
		return API::GetTextExtentSize(m_attributes.get(), str);
	}

	Size Graphics::GetTextExtent(const std::wstring& wstr, size_t length) const
	{
		return API::GetTextExtentSize(m_attributes.get(), wstr, length);
	}

	Size Graphics::GetTextExtent(const std::wstring& wstr, const Rectangle& area) const
	{
		return API::GetTextExtentSize(m_attributes.get(), area, wstr);
	}

	Size Graphics::GetTextExtent(const std::string& str, const Rectangle& area) const
	{
		return API::GetTextExtentSize(m_attributes.get(), area, str);
	}

	Size Graphics::GetTextExtent(std::wstring_view wstr) const
	{
		return API::GetTextExtentSize(m_attributes.get(), wstr);
	}

	uint32_t Graphics::GetCaretHeight() const
	{
		return API::GetCaretHeight(m_attributes.get());
	}

	void Graphics::Release()
	{
		m_attributes.reset();
		
		m_targetRT = nullptr;
		m_size = Size::Zero;
	}

	bool Graphics::IsEnabledAliasing()
	{
#ifdef BT_PLATFORM_WINDOWS
		return m_targetRT->GetAntialiasMode() == D2D1_ANTIALIAS_MODE_ALIASED;
#else
		return false;
#endif
	}

	void Graphics::EnabledAliasing(bool enabled)
	{
#ifdef BT_PLATFORM_WINDOWS
		m_targetRT->SetAntialiasMode(enabled ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#endif
	}

#ifdef BT_PLATFORM_WINDOWS
	ID2D1PathGeometry* Graphics::CreateTopRoundedGeometry(const Rectangle& rect, float radius, bool closeFigure) const
	{
		ID2D1PathGeometry* pathGeometry = nullptr;
		HRESULT hr = DirectX::D2DModule::GetInstance().GetFactory()->CreatePathGeometry(&pathGeometry);
		
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink* sink = nullptr;
			if (SUCCEEDED(pathGeometry->Open(&sink)))
			{
				float left = static_cast<float>(rect.X) + 0.5f; 
				float top = static_cast<float>(rect.Y) + 0.5f;
				float right = static_cast<float>(rect.X + rect.Width) - 0.5f;
				float bottom = static_cast<float>(rect.Y + rect.Height) - 0.5f;

				sink->BeginFigure(D2D1::Point2F(left, bottom), D2D1_FIGURE_BEGIN_FILLED);
				sink->AddLine(D2D1::Point2F(left, top + radius));
				sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(left + radius, top), D2D1::SizeF(radius, radius), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				sink->AddLine(D2D1::Point2F(right - radius, top));
				sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(right, top + radius), D2D1::SizeF(radius, radius), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				sink->AddLine(D2D1::Point2F(right, bottom));
				
				sink->EndFigure(closeFigure ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
				sink->Close();
				sink->Release();
			}
		}
		return pathGeometry;
	}

	ID2D1PathGeometry* Graphics::CreateBottomRoundedGeometry(const Rectangle& rect, float radius, bool closeFigure) const
	{
		ID2D1PathGeometry* pathGeometry = nullptr;
		HRESULT hr = DirectX::D2DModule::GetInstance().GetFactory()->CreatePathGeometry(&pathGeometry);
		
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink* sink = nullptr;
			if (SUCCEEDED(pathGeometry->Open(&sink)))
			{
				float left = static_cast<float>(rect.X) + 0.5f;
				float top = static_cast<float>(rect.Y) + 0.5f;
				float right = static_cast<float>(rect.X + rect.Width) - 0.5f;
				float bottom = static_cast<float>(rect.Y + rect.Height) - 0.5f;

				// 1. Empezamos en la esquina SUPERIOR DERECHA
				sink->BeginFigure(D2D1::Point2F(right, top), D2D1_FIGURE_BEGIN_FILLED);

				// 2. Línea derecha hacia abajo, hasta el inicio de la curva
				sink->AddLine(D2D1::Point2F(right, bottom - radius));

				// 3. Arco inferior derecho
				sink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(right - radius, bottom), // Punto final del arco
					D2D1::SizeF(radius, radius),
					0.0f, 
					D2D1_SWEEP_DIRECTION_CLOCKWISE, 
					D2D1_ARC_SIZE_SMALL
				));

				// 4. Línea horizontal inferior hacia la izquierda
				sink->AddLine(D2D1::Point2F(left + radius, bottom));

				// 5. Arco inferior izquierdo
				sink->AddArc(D2D1::ArcSegment(
					D2D1::Point2F(left, bottom - radius), // Punto final del arco
					D2D1::SizeF(radius, radius), 
					0.0f, 
					D2D1_SWEEP_DIRECTION_CLOCKWISE, 
					D2D1_ARC_SIZE_SMALL
				));

				// 6. Línea izquierda vertical hacia arriba
				sink->AddLine(D2D1::Point2F(left, top));

				// 7. Cierre. Si closeFigure es 'false', la línea que conecta 
				// Top-Left con Top-Right (la horizontal superior) NO se dibuja.
				sink->EndFigure(closeFigure ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
			
				sink->Close();
				sink->Release();
			}
		}
		return pathGeometry;
	}
#endif

	void Graphics::SetTransform(const Rectangle& area)
	{
#ifdef BT_PLATFORM_WINDOWS
		m_targetRT->SetTransform(D2D1::Matrix3x2F::Translation(static_cast<float>(area.X), static_cast<float>(area.Y)));

		m_size.Width = area.Width;
		m_size.Height = area.Height;
#endif
	}

	void Graphics::SetClipping(const Rectangle& area) const
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_targetRT)
		{
			return;
		}

		D2D1_RECT_F clipRect = D2D1::RectF
		(
			static_cast<float>(area.X), 
			static_cast<float>(area.Y), 
			static_cast<float>(area.X + area.Width), 
			static_cast<float>(area.Y + area.Height)
		);
		m_targetRT->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#endif
	}
	
	void Graphics::EndClipping()
	{
#ifdef BT_PLATFORM_WINDOWS
		if (!m_targetRT)
		{
			return;
		}

		m_targetRT->PopAxisAlignedClip();
#endif
	}
}
