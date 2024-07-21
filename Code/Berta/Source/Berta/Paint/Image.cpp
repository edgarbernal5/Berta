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
			stbi_image_free(m_imageData);
		}
	}

	void Image::Open(const std::string& filepath)
	{
		int width, height, channels;
		unsigned char* imageData = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (imageData == nullptr)
		{
			BT_CORE_ERROR << "Failed to load image: " << filepath << std::endl;
			return;
		}

		m_attributes.reset(new NativeAttributes());

		m_attributes->m_channels = static_cast<int>(channels);
		m_attributes->m_size.Height = static_cast<uint32_t>(height);
		m_attributes->m_size.Width = static_cast<uint32_t>(width);

		//HDC hdc = GetDC(NULL);

		//HDC cdc = ::CreateCompatibleDC(hdc);
		//if (cdc == nullptr)
		//{
		//	::ReleaseDC(nullptr, hdc);
		//	BT_CORE_TRACE << "Error." << std::endl;
		//	stbi_image_free(imageData);
		//	return;
		//}

		// Create a HBITMAP from the loaded image data
		/*::BITMAPINFO bmpInfo;
		ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = width;
		bmpInfo.bmiHeader.biHeight = -height; // negative to indicate top-down DIB
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = 8 * channels;
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		*/
		//m_attributes->m_hdc = cdc;
		//m_attributes->hBitmap = ::CreateDIBSection(hdc, &bmpInfo, DIB_RGB_COLORS, (void**)&imageData, NULL, 0);
		//if (!m_attributes->hBitmap)
		//{
		//	BT_CORE_ERROR << "Failed to create DIB section" << std::endl;
		//	stbi_image_free(imageData);
		//	return;
		//}

		m_imageData = imageData;
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination)
	{
		Paste(Rectangle{ m_attributes->m_size }, destination, positionDestination);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination)
	{
		HDC& destDC = destination.m_attributes->m_hdc;

		BITMAPINFO bmi;
		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = static_cast<LONG>(m_attributes->m_size.Width);
		bmi.bmiHeader.biHeight = -static_cast<LONG>(m_attributes->m_size.Height);  // top-down image
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		//bmi.bmiHeader.biSizeImage = static_cast<DWORD>(m_attributes->m_size.Width * m_attributes->m_size.Height * sizeof(pixel_color_t));;
		//bmi.bmiHeader.biSizeImage = 0;

		// Set up alpha blending
		BLENDFUNCTION blendFunc;
		blendFunc.BlendOp = AC_SRC_OVER;
		blendFunc.BlendFlags = 0;
		blendFunc.SourceConstantAlpha = 255;
		blendFunc.AlphaFormat = AC_SRC_ALPHA;

		/*
		// Create a compatible DC and bitmap
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateDIBSection(hMemDC, &bmpInfo, DIB_RGB_COLORS, (void**)&imageData, NULL, 0);

        if (hBitmap) {
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

            // Set up alpha blending
            BLENDFUNCTION blendFunc;
            blendFunc.BlendOp = AC_SRC_OVER;
            blendFunc.BlendFlags = 0;
            blendFunc.SourceConstantAlpha = 255;
            blendFunc.AlphaFormat = AC_SRC_ALPHA;

            // Apply alpha blending
            AlphaBlend(hdc, x, y, width, height, hMemDC, 0, 0, width, height, blendFunc);

            SelectObject(hMemDC, hOldBitmap);
            DeleteObject(hBitmap);
        }

        DeleteDC(hMemDC);
		*/

		::SetDIBitsToDevice(destDC,
			positionDestination.X, positionDestination.Y, 
			sourceRect.Width, sourceRect.Height,
			sourceRect.X, sourceRect.Y, 0, m_attributes->m_size.Height,
			m_imageData, &bmi,
			DIB_RGB_COLORS);

	}
}