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

	Size BasicImageAttributes::GetSize() const
	{
		return m_size;
	}

	void BasicImageAttributes::Open(const std::wstring& filepath)
	{
		auto filepathUtf = StringUtils::WideToUTF8(filepath);
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

		stbi_image_free(imageData);
	}

	void BasicImageAttributes::OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
	{
		m_size = { width, height };
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
		}
	}

	void BasicImageAttributes::Paste(Graphics& destination, const Point& positionDestination)
	{
	}

	void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect)
	{
		//m_colorBuffer.Paste(sourceRect, destination.GetHandle(), destinationRect);

		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionRect(sourceRect, GetSize(), destinationRect, destination.GetSize(), validSourceDest, validDestRect))
		{
			return;
		}

		auto handle = destination.GetHandle();
		HWND currentHwnd = handle->NativeHandle.Handle;
		
		auto& bitmapPtr = m_bitmapCache[currentHwnd];
		if (!bitmapPtr)
		{
			HRESULT hr = handle->RenderTarget->CreateBitmap
			(
				D2D1::SizeU(m_size.Width, m_size.Height),
				static_cast<void*>(m_colorBuffer.m_storage->m_buffer),
				m_colorBuffer.m_storage->m_bytesPerLine,
				D2D1::BitmapProperties
				(
					D2D1::PixelFormat
					(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						D2D1_ALPHA_MODE_PREMULTIPLIED
					)
				),
				&bitmapPtr
			);

			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Failed to create bitmap: " << std::endl;
				return;
			}
		}

		handle->RenderTarget->DrawBitmap
		(
			bitmapPtr.Get(),
			validDestRect,
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			validSourceDest
		);
	}

	void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination)
	{
		//m_colorBuffer.Paste(sourceRect, destination.GetHandle(), positionDestination);

		auto destinationSize = destination.GetSize();
		Rectangle destinationRect {positionDestination.X, positionDestination.Y, destinationSize.Width, destinationSize.Height};
		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionRect(sourceRect, GetSize(), destinationRect, destinationSize, validSourceDest, validDestRect))
		{
			return;
		}

		auto handle = destination.GetHandle();
		HWND currentHwnd = handle->NativeHandle.Handle;
		
		auto& bitmapPtr = m_bitmapCache[currentHwnd];
		if (!bitmapPtr)
		{
			HRESULT hr = handle->RenderTarget->CreateBitmap
			(
				D2D1::SizeU(m_size.Width, m_size.Height),
				static_cast<void*>(m_colorBuffer.m_storage->m_buffer),
				m_colorBuffer.m_storage->m_bytesPerLine,
				D2D1::BitmapProperties
				(
					D2D1::PixelFormat
					(
						DXGI_FORMAT_B8G8R8A8_UNORM,
						D2D1_ALPHA_MODE_PREMULTIPLIED
					)
				),
				&bitmapPtr
			);

			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Failed to create bitmap: " << std::endl;
				return;
			}
		}

		handle->RenderTarget->DrawBitmap
		(
			bitmapPtr.Get(),
			validDestRect,
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			validSourceDest
		);
	}

	void BasicImageAttributes::ReleaseNativeObjects()
	{
#if BT_PLATFORM_WINDOWS
		m_bitmapCache.clear();
#endif
	}
}