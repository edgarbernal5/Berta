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

	void BasicImageAttributes::Open(const std::string& filepath)
	{
		int width, height, channels;
		unsigned char* imageData = stbi_load(filepath.c_str(), &width, &height, &channels, 0); // Don't force RGBA
		if (imageData == nullptr)
		{
			BT_CORE_ERROR << "Failed to load image: " << filepath << std::endl;
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

		/*if (m_hasTransparency)
		{
		}
		else
		{
			auto totalBytes = static_cast<size_t>(width * height * 3);
			m_imageData = new unsigned char[totalBytes];
			for (size_t i = 0; i < totalBytes; i += 3)
			{
				m_imageData[i] = imageData[i + 2];
				m_imageData[i + 1] = imageData[i + 1];
				m_imageData[i + 2] = imageData[i];
			}
		}*/

		stbi_image_free(imageData);
	}

	void BasicImageAttributes::Paste(Graphics& destination, const Point& positionDestination)
	{
	}

	void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect)
	{
		//m_colorBuffer.Paste(sourceRect, destination.GetHandle(), destinationRect);

		Rectangle validDestRect, validSourceDest;
		if (!LayoutUtils::GetIntersectionClipRect(sourceRect, GetSize(), destinationRect, destination.GetSize(), validSourceDest, validDestRect))
		{
			return;
		}

		if (m_bitmap)
		{
			m_bitmap->Release();
			m_bitmap = nullptr;
		}

		auto handle = destination.GetHandle();
		if (!m_bitmap)
		{
			HRESULT hr = handle->m_bitmapRT->CreateBitmap
			(
				D2D1::SizeU(m_size.Width, m_size.Height),
				static_cast<void*>( m_colorBuffer.m_storage->m_buffer),                     // Pointer to your image array
				m_colorBuffer.m_storage->m_bytesPerLine,                        // Bytes per row (width * 4 for 32bpp)
				D2D1::BitmapProperties(
					D2D1::PixelFormat(
						DXGI_FORMAT_B8G8R8A8_UNORM, // Or DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
						D2D1_ALPHA_MODE_PREMULTIPLIED // Important for transparency
					)
				),
				&m_bitmap
			);

			if (FAILED(hr))
			{
				BT_CORE_ERROR << "Failed to create bitmap: " << std::endl;
				return;
			}
		}

		handle->m_bitmapRT->DrawBitmap
		(
			m_bitmap,
			validDestRect,
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			validSourceDest
		);
	}

	void BasicImageAttributes::ReleaseNativeObjects()
	{
#if BT_PLATFORM_WINDOWS
		if (m_bitmap)
		{
			m_bitmap->Release();
			m_bitmap = nullptr;
		}
#endif
	}
}