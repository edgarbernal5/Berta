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

namespace Berta
{
	BasicImageAttributes::BasicImageAttributes()
	{
	}

	BasicImageAttributes::~BasicImageAttributes()
	{
		if (m_imageData)
		{
			delete[] m_imageData;
			m_imageData = nullptr;
		}

#if BT_PLATFORM_WINDOWS
		if (m_hBitmap)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = nullptr;
		}
		if (m_hdc)
		{
			::DeleteDC(m_hdc);
			m_hdc = nullptr;
		}
#endif
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

		m_channels = channels;
		m_size.Height = static_cast<uint32_t>(height);
		m_size.Width = static_cast<uint32_t>(width);

		if (m_hasTransparency)
		{
			auto totalBytes = static_cast<size_t>(width * height * 4);
			m_imageData = new unsigned char[totalBytes];
			for (size_t i = 0; i < totalBytes; i += 4)
			{
				m_imageData[i] = imageData[i + 2];
				m_imageData[i + 1] = imageData[i + 1];
				m_imageData[i + 2] = imageData[i];
				m_imageData[i + 3] = imageData[i + 3];
			}
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
		}

		stbi_image_free(imageData);
	}

	void BasicImageAttributes::Paste(Graphics& destination, const Point& positionDestination)
	{
	}

	void BasicImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect)
	{
	}
}