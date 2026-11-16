/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ExternalTextureAttributes.h"

#include "Berta/Paint/Graphics.h"

namespace Berta
{
	ExternalTextureAttributes::ExternalTextureAttributes(const ExternalTextureDesc& desc)
	{
		Update(desc);
	}

	ExternalTextureAttributes::~ExternalTextureAttributes()
	{
		ReleaseNativeObjects();
	}

	void ExternalTextureAttributes::Update(const ExternalTextureDesc& desc)
	{
		ReleaseNativeObjects();
		m_desc = desc;

		float scale = (m_desc.DpiScale > 0.0f) ? m_desc.DpiScale : 1.0f;
		m_logicalSize = {
			static_cast<uint32_t>(static_cast<float>(m_desc.Width) / scale),
			static_cast<uint32_t>(static_cast<float>(m_desc.Height) / scale)
		};
	}

	Size ExternalTextureAttributes::GetLogicalSize() const
	{
		return m_logicalSize;
	}

	void ExternalTextureAttributes::Paste(Graphics& destination, const Point& positionDestination, const ImageDrawOptions& options)
	{
		Size logicalSize = GetLogicalSize();
		DrawInternal(logicalSize.ToRectangle(), destination, { positionDestination.X, positionDestination.Y, logicalSize.Width, logicalSize.Height }, options);
	}

	void ExternalTextureAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination, const ImageDrawOptions& options)
	{
		Size destinationSize = destination.GetSize();
		Rectangle destinationRect{ positionDestination.X, positionDestination.Y, destinationSize.Width, destinationSize.Height };
		DrawInternal(sourceRect, destination, destinationRect, options);
	}

	void ExternalTextureAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect, const ImageDrawOptions& options)
	{
		DrawInternal(sourceRect, destination, destinationRect, options);
	}

	void ExternalTextureAttributes::DrawInternal(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect, const ImageDrawOptions& options)
	{
		if (!m_desc.NativeHandle || m_logicalSize.Width == 0 || m_logicalSize.Height == 0)
		{
			return;
		}

		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionRect(sourceRect, m_logicalSize, destinationRect, destination.GetSize(), validSourceDest, validDestRect))
		{
			return;
		}

#if BT_PLATFORM_WINDOWS
		auto* handle = destination.GetHandle();
		if (!handle)
		{
			return;
		}

		ID2D1RenderTarget* activeRT = destination.m_targetRT ? destination.m_targetRT : handle->RenderTarget;
		if (!activeRT)
		{
			return;
		}

		HWND hwnd = handle->NativeHandle.Handle;
		ID2D1Bitmap* sharedBitmap = GetOrCreateSharedBitmap(hwnd, activeRT);
		if (!sharedBitmap)
		{
			return;
		}

		if (m_desc.OnBeforeDraw)
		{
			m_desc.OnBeforeDraw();
		}

		float scaleX = static_cast<float>(m_desc.Width) / static_cast<float>(m_logicalSize.Width);
		float scaleY = static_cast<float>(m_desc.Height) / static_cast<float>(m_logicalSize.Height);

		D2D1_RECT_F d2dSourceRect = D2D1::RectF(
			static_cast<FLOAT>(validSourceDest.X * scaleX),
			static_cast<FLOAT>(validSourceDest.Y * scaleY),
			static_cast<FLOAT>((validSourceDest.X + validSourceDest.Width) * scaleX),
			static_cast<FLOAT>((validSourceDest.Y + validSourceDest.Height) * scaleY)
		);

		D2D1_RECT_F d2dDestRect = D2D1::RectF(
			static_cast<FLOAT>(validDestRect.X),
			static_cast<FLOAT>(validDestRect.Y),
			static_cast<FLOAT>(validDestRect.X + validDestRect.Width),
			static_cast<FLOAT>(validDestRect.Y + validDestRect.Height)
		);

		D2D1_BITMAP_INTERPOLATION_MODE interpMode = (options.Filter == TextureFilterMode::NearestNeighbor)
			? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
			: D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;

		activeRT->DrawBitmap(
			sharedBitmap,
			&d2dDestRect,
			1.0f,
			interpMode,
			&d2dSourceRect
		);

		if (m_desc.OnAfterDraw)
		{
			m_desc.OnAfterDraw();
		}
#endif
	}

#if BT_PLATFORM_WINDOWS
	ID2D1Bitmap* ExternalTextureAttributes::GetOrCreateSharedBitmap(HWND hwnd, ID2D1RenderTarget* renderTarget)
	{
		auto& cachedBitmap = m_bitmapCache[hwnd];
		if (cachedBitmap)
		{
			return cachedBitmap.Get();
		}

		auto* dxgiSurface = static_cast<IDXGISurface*>(m_desc.NativeHandle);
		D2D1_ALPHA_MODE alphaMode = (m_desc.AlphaMode == TextureAlphaMode::Premultiplied)
			? D2D1_ALPHA_MODE_PREMULTIPLIED
			: D2D1_ALPHA_MODE_IGNORE;

		float dpi = 96.0f * ((m_desc.DpiScale > 0.0f) ? m_desc.DpiScale : 1.0f);

		D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, alphaMode),
			dpi,
			dpi
		);

		HRESULT hr = renderTarget->CreateSharedBitmap(
			__uuidof(IDXGISurface),
			dxgiSurface,
			&bitmapProps,
			&cachedBitmap
		);

		if (FAILED(hr))
		{
			BT_CORE_ERROR << "ExternalTextureAttributes: Failed to create shared D2D bitmap. HRESULT: 0x"
						  << std::hex << hr << std::dec << std::endl;
			cachedBitmap.Reset();
			return nullptr;
		}

		return cachedBitmap.Get();
	}
#endif

	void ExternalTextureAttributes::ReleaseNativeObjects()
	{
#if BT_PLATFORM_WINDOWS
		m_bitmapCache.clear();
#endif
	}
}