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
#if BT_PLATFORM_WINDOWS
		HDC& destDC = destination.m_attributes->m_hdc;
		
		//float scaleFactor = m_hasTransparency ? 1.0f : LayoutUtils::CalculateDPIScaleFactor(currentDpi);

		//int adjustedWidth = static_cast<int>(m_size.Width * scaleFactor);
		//int adjustedHeight = static_cast<int>(m_size.Height * scaleFactor);

		HDC hdc = ::GetDC(NULL);
		// Create compatible DC
		HDC hdcMem = ::CreateCompatibleDC(hdc);

		::BITMAPINFO bmpInfo;
		ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = static_cast<LONG>(destinationRect.Width);
		bmpInfo.bmiHeader.biHeight = -static_cast<LONG>(destinationRect.Height);  // top-down image
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = m_channels * 8;
		bmpInfo.bmiHeader.biCompression = BI_RGB;

		// Allocate memory for bitmap data
		void* pBits;
		HBITMAP hBitmap = ::CreateDIBSection(hdcMem, &bmpInfo, DIB_RGB_COLORS, &pBits, NULL, 0);
		if (!hBitmap)
		{
			BT_CORE_ERROR << "Failed to create DIB section." << std::endl;
			return;
		}

		if (m_hasTransparency)
		{
			//if (enabled)
			{
				//TODO: DONT KNOW WHY I HAVE TO CALL THIS!
				memcpy(pBits, m_imageData, m_size.Width * m_size.Height * 4);
			}
			/*else
			{
				auto pBitsChar = (unsigned char*)pBits;
				for (size_t i = 0; i < m_size.Width * m_size.Height * 4; i += 4)
				{
					// Calculate luminosity value
					unsigned char gray = static_cast<unsigned char>(0.2126f * m_imageData[i + 2] + 0.7152f * m_imageData[i + 1] + 0.0722f * m_imageData[i]);
					pBitsChar[i] = pBitsChar[i + 1] = pBitsChar[i + 2] = gray;

					pBitsChar[i + 3] = m_imageData[i + 3];
				}
			}
			*/
		}

		m_hBitmap = hBitmap;
		m_hdc = hdcMem;

		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(m_hdc, m_hBitmap);
		if (m_hasTransparency)
		{
			// Set up alpha blending
			BLENDFUNCTION blendFunc;
			blendFunc.BlendOp = AC_SRC_OVER;
			blendFunc.BlendFlags = 0;
			blendFunc.SourceConstantAlpha = 255;
			blendFunc.AlphaFormat = AC_SRC_ALPHA;

			// Apply alpha blending
			if (!::AlphaBlend(destDC, destinationRect.X, destinationRect.Y,
				(int)(destinationRect.Width), (int)(destinationRect.Height), m_hdc,
				sourceRect.X, sourceRect.Y,
				m_size.Width, m_size.Height, blendFunc))
			{
				BT_CORE_ERROR << "Failed to Alpha Blend. ::GetLastError() = " << GetLastError() << std::endl;
			}
		}
		else
		{
			int adjustedWidth = static_cast<int>(destinationRect.Width);
			int adjustedHeight = static_cast<int>(destinationRect.Height);

			::BITMAPINFO bmpInfo;
			ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmpInfo.bmiHeader.biWidth = static_cast<LONG>(m_size.Width);
			bmpInfo.bmiHeader.biHeight = -static_cast<LONG>(m_size.Height);
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = m_channels * 8;
			bmpInfo.bmiHeader.biCompression = BI_RGB;

			// Stretch the image data to the scaled bitmap
			if (::StretchDIBits(m_hdc, 0, 0, adjustedWidth, adjustedHeight, 
				0, 0, (int)m_size.Width, (int)m_size.Height,
				m_imageData,
				&bmpInfo, DIB_RGB_COLORS, SRCCOPY) == 0)
			{
				BT_CORE_ERROR << "Failed to strech DIB bits. ::GetLastError() = " << std::endl;
			}

			// Render the scaled bitmap
			if (!::BitBlt(destDC, destinationRect.X, destinationRect.Y,
				adjustedWidth, adjustedHeight, m_hdc, 0, 0, SRCCOPY))
			{
				BT_CORE_ERROR << " - BitBlt ::GetLastError() = " << ::GetLastError() << std::endl;
			}
		}

		//m_enabled = enabled;
		::ReleaseDC(0, hdc);

#endif
	}
}