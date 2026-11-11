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
		
		/*auto filepathUtf = StringUtils::WideToUTF8(filepath);
		int width, height, channels;
		unsigned char* imageData = stbi_load(filepathUtf.c_str(), &width, &height, &channels, 0); // Don't force RGBA
		if (imageData == nullptr)
		{
			BT_CORE_ERROR << "Failed to load image: " << filepathUtf << std::endl;
			return;
		}

		m_hasTransparency = channels == 4;
		uint32_t bitsPerPixel = channels * 8;

		m_channels = channels;
		m_size.Height = static_cast<uint32_t>(height);
		m_size.Width = static_cast<uint32_t>(width);

		if (m_hasTransparency)
		{
			auto totalBytes = static_cast<size_t>(width * height * 4);
			for (size_t i = 0; i < totalBytes; i += 4)
			{
				uint8_t i0 = imageData[i];
				uint8_t i1 = imageData[i + 1];
				uint8_t i2 = imageData[i + 2];
				uint8_t i3 = imageData[i + 3];
				imageData[i] = i2;
				imageData[i + 1] = i1;
				imageData[i + 2] = i0;
				imageData[i + 3] = i3;
			}

			//Premultiplied alpha.
			for (size_t i = 0; i < totalBytes; i += 4)
			{
				unsigned char* pixel = &imageData[i];
				float alpha = pixel[3] / 255.0f;
				pixel[0] = static_cast<unsigned char>(pixel[0] * alpha);
				pixel[1] = static_cast<unsigned char>(pixel[1] * alpha);
				pixel[2] = static_cast<unsigned char>(pixel[2] * alpha);
			}
		}

		m_colorBuffer.Create(m_size);
		m_colorBuffer.SetAlphaChannel(m_hasTransparency);
		m_colorBuffer.Copy(imageData, m_size.Width, m_size.Height, bitsPerPixel, m_size.Width * channels);

		stbi_image_free(imageData);*/
	}

	void BasicImageAttributes::OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
	{
		AddVariantFromMemory(1.0f, pixels, width, height, channels);
		
		/*m_size = { width, height };
		m_channels = channels;
		m_hasTransparency = (channels == 4);
		uint32_t bitsPerPixel = channels * 8;

		m_colorBuffer.Create(m_size);
		m_colorBuffer.SetAlphaChannel(m_hasTransparency);
		m_colorBuffer.Copy(pixels, width, height, bitsPerPixel, width * channels);

		// Swizzle a BGRA y Premultiplicación Alfa en una sola pasada (KISS)
		if (m_hasTransparency)
		{
			auto totalBytes = static_cast<size_t>(width * height * 4);
			auto* buffer = reinterpret_cast<uint8_t*>(m_colorBuffer.m_storage->m_buffer);
        
			for (size_t i = 0; i < totalBytes; i += 4)
			{
				uint8_t r = buffer[i];
				uint8_t g = buffer[i + 1];
				uint8_t b = buffer[i + 2];
				uint8_t a = buffer[i + 3];

				float alpha = a / 255.0f;
				buffer[i]     = static_cast<uint8_t>(b * alpha);
				buffer[i + 1] = static_cast<uint8_t>(g * alpha);
				buffer[i + 2] = static_cast<uint8_t>(r * alpha);
				// El canal alfa (buffer[i + 3]) se mantiene igual
			}
		}*/
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

		// Swizzle a BGRA y Premultiplicación Alfa en una pasada[cite: 1]
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

	void BasicImageAttributes::Paste(Graphics& destination, const Point& positionDestination)
	{
	}

	void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect)
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
		auto handle = destination.GetHandle();
		HWND hwnd = handle->NativeHandle.Handle;
		
		auto& cacheEntry = m_bitmapCache[hwnd];
		if (!cacheEntry.Bitmap || cacheEntry.CachedScale != actualScale)
		{
			cacheEntry.Bitmap.Reset();
			Size actualSize = colorBuffer->GetSize();
			
			HRESULT hr = handle->RenderTarget->CreateBitmap
			(
				D2D1::SizeU(actualSize.Width, actualSize.Height),
				static_cast<void*>(colorBuffer->m_storage->m_buffer),
				colorBuffer->m_storage->m_bytesPerLine,
				D2D1::BitmapProperties
				(
					D2D1::PixelFormat
					(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						D2D1_ALPHA_MODE_PREMULTIPLIED
					)
				),
				&cacheEntry.Bitmap
			);

			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Failed to create bitmap: " << std::endl;
				return;
			}
			cacheEntry.CachedScale = actualScale;
		}
		
		float scaleX = static_cast<float>(colorBuffer->GetSize().Width) / static_cast<float>(m_logicalSize.Width);
		float scaleY = static_cast<float>(colorBuffer->GetSize().Height) / static_cast<float>(m_logicalSize.Height);

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

		handle->RenderTarget->DrawBitmap
		(
			cacheEntry.Bitmap.Get(),
			&d2dDestRect,
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			&d2dSourceRect
		);
#endif
	}

	void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination)
	{
		auto destinationSize = destination.GetSize();
		Rectangle destinationRect {positionDestination.X, positionDestination.Y, destinationSize.Width, destinationSize.Height};
		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionRect(sourceRect, GetLogicalSize(), destinationRect, destinationSize, validSourceDest, validDestRect))
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
		auto handle = destination.GetHandle();
		HWND hwnd = handle->NativeHandle.Handle;
		
		auto& cacheEntry = m_bitmapCache[hwnd];
		if (!cacheEntry.Bitmap || cacheEntry.CachedScale != actualScale)
		{
			cacheEntry.Bitmap.Reset();
			Size actualSize = colorBuffer->GetSize();
			HRESULT hr = handle->RenderTarget->CreateBitmap
			(
				D2D1::SizeU(actualSize.Width, actualSize.Height),
				static_cast<void*>(colorBuffer->m_storage->m_buffer),
				colorBuffer->m_storage->m_bytesPerLine,
				D2D1::BitmapProperties
				(
					D2D1::PixelFormat
					(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						D2D1_ALPHA_MODE_PREMULTIPLIED
					)
				),
				&cacheEntry.Bitmap
			);

			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Failed to create bitmap: " << std::endl;
				return;
			}
			cacheEntry.CachedScale = actualScale;
		}
		
		float scaleX = static_cast<float>(colorBuffer->GetSize().Width) / static_cast<float>(m_logicalSize.Width);
		float scaleY = static_cast<float>(colorBuffer->GetSize().Height) / static_cast<float>(m_logicalSize.Height);
		
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

		handle->RenderTarget->DrawBitmap
		(
			cacheEntry.Bitmap.Get(),
			&d2dDestRect,
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
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
