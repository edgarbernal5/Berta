/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "BasicImageAttributes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#if BT_PLATFORM_WINDOWS
#pragma comment(lib, "Msimg32.lib")
#endif

#include "Berta/Paint/Graphics.h"

namespace Berta
{
	BasicImageAttributes::BasicImageAttributes()
	{
	}

	BasicImageAttributes::~BasicImageAttributes()
	{
		ReleaseNativeObjects();
	}

	Size BasicImageAttributes::GetLogicalSize() const
	{
		return m_logicalSize;
	}

	void BasicImageAttributes::Open(const std::wstring& filepath)
	{
		AddVariant(1.0f, filepath);
	}

	void BasicImageAttributes::OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
	{
		AddVariantFromMemory(1.0f, pixels, width, height, channels);
	}

	void BasicImageAttributes::UpdateFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels, uint32_t stride)
	{
		AddVariantFromMemory(1.0f, pixels, width, height, channels);

#if BT_PLATFORM_WINDOWS
		for (auto& [hwnd, entry] : m_bitmapCache)
		{
			entry.IsDirty = true;
		}
#endif
	}

	void BasicImageAttributes::AddVariant(float scaleFactor, const std::wstring& filepath)
	{
		// Convertir std::wstring a std::string para stbi_load
		std::string path(filepath.begin(), filepath.end());
		int width, height, channels;
		unsigned char* imageData = stbi_load(path.c_str(), &width, &height, &channels, 0);
            
		if (imageData)
		{
			AddVariantFromMemory(scaleFactor, imageData, width, height, channels);
			stbi_image_free(imageData);
		}
		else
		{
			BT_CORE_ERROR << "Failed to load variant: " << path << std::endl;
		}
	}

	void BasicImageAttributes::AddVariantFromMemory(float scaleFactor, const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
	{
		ColorBuffer buffer;
		buffer.Create({ width, height });
		buffer.SetAlphaChannel(channels == 4);
		buffer.Copy(pixels, width, height, channels * 8, width * channels);

		// Swizzle a BGRA y Premultiplicación Alfa en una pasada
		if (channels == 4)
		{
			auto totalBytes = static_cast<size_t>(width * height * 4);
			auto* bufPtr = reinterpret_cast<uint8_t*>(buffer.m_storage->m_buffer);
			for (size_t i = 0; i < totalBytes; i += 4)
			{
				uint8_t r = bufPtr[i];
				uint8_t g = bufPtr[i + 1];
				uint8_t b = bufPtr[i + 2];
				uint8_t a = bufPtr[i + 3];

				float alpha = a / 255.0f;
				bufPtr[i]     = static_cast<uint8_t>(b * alpha);
				bufPtr[i + 1] = static_cast<uint8_t>(g * alpha);
				bufPtr[i + 2] = static_cast<uint8_t>(r * alpha);
			}
		}

		m_variants[scaleFactor] = std::move(buffer);

		// La escala 1.0f dicta el tamaño lógico oficial para el Layout
		if (scaleFactor == 1.0f || m_logicalSize.Width == 0)
		{
			m_logicalSize = { 
				static_cast<uint32_t>(static_cast<float>(width) / scaleFactor), 
				static_cast<uint32_t>(static_cast<float>(height) / scaleFactor) 
			};
		}
	}

	void BasicImageAttributes::Paste(Graphics& destination, const Point& positionDestination, const ImageDrawOptions& options)
{
	Size logicalSize = GetLogicalSize();
	DrawInternal(logicalSize.ToRectangle(), destination, { positionDestination.X, positionDestination.Y, logicalSize.Width, logicalSize.Height }, options);
}

void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect, const ImageDrawOptions& options)
{
	DrawInternal(sourceRect, destination, destinationRect, options);
}

void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination, const ImageDrawOptions& options)
{
	auto destinationSize = destination.GetSize();
	Rectangle destinationRect{ positionDestination.X, positionDestination.Y, destinationSize.Width, destinationSize.Height };
	DrawInternal(sourceRect, destination, destinationRect, options);
}

void BasicImageAttributes::DrawInternal(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect, const ImageDrawOptions& options)
	{
		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionRect(sourceRect, GetLogicalSize(), destinationRect, destination.GetSize(), validSourceDest, validDestRect))
		{
			return;
		}

		float currentScale = destination.GetDpiScale();
		auto [actualScale, colorBuffer] = GetBestBuffer(currentScale);
		if (!colorBuffer)
		{
			return;
		}

#if BT_PLATFORM_WINDOWS
		auto* handle = destination.GetHandle();
		ID2D1RenderTarget* activeRT = destination.m_targetRT ? destination.m_targetRT : handle->RenderTarget;
		HWND hwnd = handle->NativeHandle.Handle;

		auto& cacheEntry = m_bitmapCache[hwnd];
		Size actualSize = colorBuffer->GetSize();

		bool sizeChanged = (cacheEntry.CachedSize.Width != actualSize.Width || cacheEntry.CachedSize.Height != actualSize.Height);
		if (!cacheEntry.Bitmap || cacheEntry.CachedScale != actualScale || sizeChanged)
		{
			cacheEntry.Bitmap.Reset();
			HRESULT hr = activeRT->CreateBitmap(
				D2D1::SizeU(actualSize.Width, actualSize.Height),
				static_cast<void*>(colorBuffer->m_storage->m_buffer),
				colorBuffer->m_storage->m_bytesPerLine,
				D2D1::BitmapProperties(
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
				),
				&cacheEntry.Bitmap
			);

			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Failed to create bitmap." << std::endl;
				return;
			}
			cacheEntry.CachedScale = actualScale;
			cacheEntry.CachedSize = actualSize;
			cacheEntry.IsDirty = false;
		}
		else if (cacheEntry.IsDirty)
		{
			D2D1_RECT_U copyRect = D2D1::RectU(0, 0, actualSize.Width, actualSize.Height);
			cacheEntry.Bitmap->CopyFromMemory(
				&copyRect,
				static_cast<const void*>(colorBuffer->m_storage->m_buffer),
				colorBuffer->m_storage->m_bytesPerLine
			);
			cacheEntry.IsDirty = false;
		}

		float scaleX = static_cast<float>(actualSize.Width) / static_cast<float>(m_logicalSize.Width);
		float scaleY = static_cast<float>(actualSize.Height) / static_cast<float>(m_logicalSize.Height);

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
			cacheEntry.Bitmap.Get(),
			&d2dDestRect,
			1.0f,
			interpMode,
			&d2dSourceRect
		);
#endif
	}

	std::pair<float, const ColorBuffer*> BasicImageAttributes::GetBestBuffer(float targetScale) const
	{
		if (m_variants.empty())
		{
			return { 1.0f, nullptr };
		}
		
		auto it = m_variants.lower_bound(targetScale);
		if (it != m_variants.end())
		{
			return { it->first, &(it->second) };
		}

		return { m_variants.rbegin()->first, &(m_variants.rbegin()->second) };
	}

	void BasicImageAttributes::ReleaseNativeObjects()
	{
#if BT_PLATFORM_WINDOWS
		m_bitmapCache.clear();
#endif
	}
}
