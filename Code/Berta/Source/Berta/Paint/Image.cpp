/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Image.h"

#include "Berta/Paint/Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#if BT_PLATFORM_WINDOWS
#pragma comment(lib, "Msimg32.lib")
#endif

namespace Berta
{
	Image::Image(const std::string& filepath)
	{
		Open(filepath);
	}

	Image::~Image()
	{
		if (m_imageData)
		{
			m_imageData = nullptr;
		}
		if (m_attributes)
		{
			if (m_attributes->hBitmap)
			{
				::DeleteObject(m_attributes->hBitmap);
				m_attributes->hBitmap = nullptr;
			}
			if (m_attributes->m_hdc)
			{
				::DeleteDC(m_attributes->m_hdc);
				m_attributes->m_hdc = nullptr;
			}
		}
	}

	void Image::Open(const std::string& filepath)
	{
		int width, height, channels;
		unsigned char* imageData = stbi_load(filepath.c_str(), &width, &height, &channels, 0); // Don't force RGBA
		//unsigned char* imageData = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (imageData == nullptr)
		{
			BT_CORE_ERROR << "Failed to load image: " << filepath << std::endl;
			return;
		}
		
		m_hasTransparency = channels == 4;
		m_attributes.reset(new NativeAttributes());

		m_channels = channels;
		m_size.Height = static_cast<uint32_t>(height);
		m_size.Width = static_cast<uint32_t>(width);

		BITMAPINFO bmi;
		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = static_cast<LONG>(m_size.Width);
		bmi.bmiHeader.biHeight = -static_cast<LONG>(m_size.Height);  // top-down image
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = channels * 8;
		bmi.bmiHeader.biCompression = BI_RGB;

		void* bitmapData;
		// Create a compatible DC and bitmap
		HDC hdc = GetDC(NULL);
		HDC hMemDC = ::CreateCompatibleDC(hdc);
		HBITMAP hBitmap = ::CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, (void**)&bitmapData, NULL, 0);
		if (!hBitmap)
		{
			BT_CORE_ERROR << "Failed to create DIB section." << std::endl;
			stbi_image_free(imageData);
			return;
		}

		m_imageData = (unsigned char*)bitmapData;
		if (m_hasTransparency)
		{
			auto totalBytes = static_cast<size_t>(width * height * 4);
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
			for (size_t i = 0; i < totalBytes; i += 3)
			{
				m_imageData[i] = imageData[i + 2];
				m_imageData[i + 1] = imageData[i + 1];
				m_imageData[i + 2] = imageData[i];
			}
		}

		m_attributes->hBitmap = hBitmap;
		m_attributes->m_hdc = hMemDC;

		::ReleaseDC(0, hdc);
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination)
	{
		Paste(Rectangle{ m_size }, destination, positionDestination);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination)
	{
		if (!m_attributes->hBitmap)
		{
			return;
		}
		HDC& destDC = destination.m_attributes->m_hdc;
		
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(m_attributes->m_hdc, m_attributes->hBitmap);
		if (m_hasTransparency)
		{
			// Set up alpha blending
			BLENDFUNCTION blendFunc;
			blendFunc.BlendOp = AC_SRC_OVER;
			blendFunc.BlendFlags = 0;
			blendFunc.SourceConstantAlpha = 255;
			blendFunc.AlphaFormat = AC_SRC_ALPHA;

			// Apply alpha blending
			if (!::AlphaBlend(destDC, positionDestination.X, positionDestination.Y,
				(int)m_size.Width, (int)m_size.Height, m_attributes->m_hdc,
				sourceRect.X, sourceRect.Y, m_size.Width, m_size.Height, blendFunc))
			{
				BT_CORE_ERROR << "Error en AlphaBlend. " << GetLastError() << std::endl;
			}
		}
		else
		{
			::BITMAPINFO bmpInfo;
			ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmpInfo.bmiHeader.biWidth = m_size.Width;
			bmpInfo.bmiHeader.biHeight = -static_cast<LONG>(m_size.Height); // negative to indicate top-down DIB
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = 8 * m_channels;
			bmpInfo.bmiHeader.biCompression = BI_RGB;

			::SetDIBitsToDevice(destDC,
				positionDestination.X, positionDestination.Y,
				sourceRect.Width, sourceRect.Height,
				sourceRect.X, sourceRect.Y, 0, m_size.Height,
				m_imageData, &bmpInfo,
				DIB_RGB_COLORS);
		}
		::SelectObject(m_attributes->m_hdc, hOldBitmap);
	}
}