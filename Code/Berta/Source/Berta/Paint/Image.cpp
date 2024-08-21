/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Image.h"

#include "Berta/Paint/Graphics.h"
#include "Berta/Paint/Images/IconImageAttributes.h"
#include "Berta/Paint/Images/BasicImageAttributes.h"

namespace Berta
{
	Image::Image(const std::string& filepath)
	{
		Open(filepath);
	}

	Image::Image(const Image& other) : 
		m_attributes(other.m_attributes)
	{
	}

	Image::Image(Image&& other) noexcept :
		m_attributes(std::move(other.m_attributes))
	{
	}

	Image::~Image()
	{
		if (m_attributes)
		{
			m_attributes.reset();
		}
	}

	Image::operator bool() const
	{
		return m_attributes.get();
	}

	Image& Image::operator=(const Image& rhs)
	{
		if (this != &rhs)
		{
			m_attributes = rhs.m_attributes;
			//m_size = rhs.m_size;
			//m_channels = rhs.m_channels;
			//m_hasTransparency = rhs.m_hasTransparency;
			//m_imageData = rhs.m_imageData;
			//m_isIcon = rhs.m_isIcon;
		}

		return *this;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		if (this != &other)
		{
			m_attributes = std::move(other.m_attributes);
			//m_size = std::move(other.m_size);
			//m_channels = std::move(other.m_channels);
			//m_hasTransparency = std::move(other.m_hasTransparency);
			//m_imageData = std::move(other.m_imageData);
			//m_isIcon = std::move(other.m_isIcon);
		}

		return *this;
	}

	void Image::Open(const std::string& filepath)
	{
		std::filesystem::path path{ filepath };
		if (!path.has_extension())
			return;

		if (path.extension() == ".ico")
		{
			m_attributes = std::make_shared<IconImageAttributes>();
		}
		else if (path.extension() == ".bmp" || path.extension() == ".jpeg" ||
			path.extension() == ".png")
		{
			m_attributes = std::make_shared<BasicImageAttributes>();
		}
	}

	void Image::OpenIcon(const std::string& filepath)
	{
		
	}

	void Image::CheckAndUpdateHdc(uint32_t currentDpi, bool enabled)
	{
		//if (m_lastDpi == currentDpi && m_enabled == enabled)
		//{
		//	return;
		//}
		//m_attributes.reset(new NativeAttributes());
		//float scaleFactor = m_hasTransparency ? 1.0f : LayoutUtils::CalculateDPIScaleFactor(currentDpi);

		//int adjustedWidth = static_cast<int>(m_size.Width * scaleFactor);
		//int adjustedHeight = static_cast<int>(m_size.Height * scaleFactor);

		//HDC hdc = ::GetDC(NULL);
		//// Create compatible DC
		//HDC hdcMem = ::CreateCompatibleDC(hdc);

		//::BITMAPINFO bmpInfo;
		//ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		//bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		//bmpInfo.bmiHeader.biWidth = static_cast<LONG>(adjustedWidth);
		//bmpInfo.bmiHeader.biHeight = -static_cast<LONG>(adjustedHeight);  // top-down image
		//bmpInfo.bmiHeader.biPlanes = 1;
		//bmpInfo.bmiHeader.biBitCount = m_channels * 8;
		//bmpInfo.bmiHeader.biCompression = BI_RGB;

		//// Allocate memory for bitmap data
		//void* pBits;
		//HBITMAP hBitmap = ::CreateDIBSection(hdcMem, &bmpInfo, DIB_RGB_COLORS, &pBits, NULL, 0);
		//if (!hBitmap)
		//{
		//	BT_CORE_ERROR << "Failed to create DIB section." << std::endl;
		//	return;
		//}

		//if (m_hasTransparency)
		//{
		//	if (enabled)
		//	{
		//		//TODO: DONT KNOW WHY I HAVE TO CALL THIS!
		//		memcpy(pBits, m_imageData, m_size.Width * m_size.Height * 4);
		//	}
		//	else
		//	{
		//		auto pBitsChar = (unsigned char*)pBits;
		//		for (size_t i = 0; i < m_size.Width * m_size.Height * 4; i += 4)
		//		{
		//			// Calculate luminosity value
		//			unsigned char gray = static_cast<unsigned char>(0.2126f * m_imageData[i + 2] + 0.7152f * m_imageData[i + 1] + 0.0722f * m_imageData[i]);
		//			pBitsChar[i] = pBitsChar[i + 1] = pBitsChar[i + 2] = gray;

		//			pBitsChar[i + 3] = m_imageData[i + 3];
		//		}
		//	}
		//}

		//m_attributes->m_hBitmap = hBitmap;
		//m_attributes->m_hdc = hdcMem;

		//m_lastDpi = currentDpi;
		//m_enabled = enabled;
		//::ReleaseDC(0, hdc);
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination, bool isEnabled)
	{
		//Paste(Rectangle{ m_size }, destination, positionDestination, isEnabled);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination, bool isEnabled)
	{
		if (!m_attributes)
		{
			return;
		}

#if BT_PLATFORM_WINDOWS
		HDC& destDC = destination.m_attributes->m_hdc;

		CheckAndUpdateHdc(destination.GetDpi(), isEnabled);
		
		/*
			void Image::GrayscaleFilter(HDC hdc, int width, int height)
			{
				BITMAPINFOHEADER bmi;
				memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
				bmi.biSize = sizeof(BITMAPINFOHEADER);
				bmi.biWidth = width;
				bmi.biHeight = -height; // Top-down
				bmi.biPlanes = 1;
				bmi.biBitCount = 24; // Assuming 24bpp

				void* bits;
				HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &bits, NULL, 0);
				if (!hBitmap)
				{
					// Handle error
					return;
				}

				HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

				// Iterate through pixels
				unsigned char* pixel = (unsigned char*)bits;
				for (int y = 0; y < height; ++y)
				{
					for (int x = 0; x < width; ++x)
					{
						// Calculate grayscale value (common method: average of RGB)
						unsigned char gray = (pixel[0] + pixel[1] + pixel[2]) / 3;
						pixel[0] = pixel[1] = pixel[2] = gray;
						pixel += 3; // Move to next pixel
					}
				}

				// Copy grayscale image back to the DC
				BitBlt(hdc, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

				SelectObject(hdc, hOldBitmap);
				DeleteObject(hBitmap);
			}
		*/
		/*
			void Image::LuminosityFilter(HDC hdc, int width, int height)
			{
				// ... (similar code as GrayscaleFilter)

				// Calculate luminosity value
				unsigned char gray = static_cast<unsigned char>(0.2126 * pixel[0] + 0.7152 * pixel[1] + 0.0722 * pixel[2]);
				pixel[0] = pixel[1] = pixel[2] = gray;
			}

		*/
		/*
		HICON CreateGrayscaleIcon(HICON hIcon)
		{
			// Extract icon to a bitmap
			ICONINFO iconInfo;
			if (!GetIconInfo(hIcon, &iconInfo))
				return NULL;

			BITMAP bmp;
			GetObject(iconInfo.hbmColor, sizeof(bmp), &bmp);

			// Create a compatible DC
			HDC hdcMem = CreateCompatibleDC(NULL);
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, iconInfo.hbmColor);

			// Apply grayscale effect to the bitmap (replace with your grayscale function)
			GrayscaleFilter(hdcMem, bmp.bmWidth, bmp.bmHeight);

			// Create a new icon from the modified bitmap
			HICON hIconGray = CreateIconFromBitmap(hdcMem, iconInfo.hbmMask, bmp.bmWidth, bmp.bmHeight, 0);

			// Clean up
			SelectObject(hdcMem, hbmOld);
			DeleteObject(iconInfo.hbmColor);
			DeleteObject(iconInfo.hbmMask);
			DeleteDC(hdcMem);

			return hIconGray;
		}
		*/

		//float scaleFactor = LayoutUtils::CalculateDPIScaleFactor(destination.GetDpi());
		//HBITMAP hOldBitmap = (HBITMAP)::SelectObject(m_attributes->m_hdc, m_attributes->m_hBitmap);
		//if (m_hasTransparency)
		//{
		//	// Set up alpha blending
		//	BLENDFUNCTION blendFunc;
		//	blendFunc.BlendOp = AC_SRC_OVER;
		//	blendFunc.BlendFlags = 0;
		//	blendFunc.SourceConstantAlpha = 255;
		//	blendFunc.AlphaFormat = AC_SRC_ALPHA;

		//	// Apply alpha blending
		//	if (!::AlphaBlend(destDC, positionDestination.X, positionDestination.Y,
		//		(int)(m_size.Width * scaleFactor), (int)(m_size.Height * scaleFactor), m_attributes->m_hdc,
		//		sourceRect.X, sourceRect.Y,
		//		m_size.Width, m_size.Height, blendFunc))
		//	{
		//		BT_CORE_ERROR << "Failed to Alpha Blend. ::GetLastError() = " << GetLastError() << std::endl;
		//	}
		//}
		//else
		//{
		//	int adjustedWidth = static_cast<int>(m_size.Width * scaleFactor);
		//	int adjustedHeight = static_cast<int>(m_size.Height * scaleFactor);

		//	::BITMAPINFO bmpInfo;
		//	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		//	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		//	bmpInfo.bmiHeader.biWidth = static_cast<LONG>(m_size.Width);
		//	bmpInfo.bmiHeader.biHeight = -static_cast<LONG>(m_size.Height);
		//	bmpInfo.bmiHeader.biPlanes = 1;
		//	bmpInfo.bmiHeader.biBitCount = m_channels * 8;
		//	bmpInfo.bmiHeader.biCompression = BI_RGB;

		//	// Stretch the image data to the scaled bitmap
		//	if (::StretchDIBits(m_attributes->m_hdc, 0, 0, adjustedWidth, adjustedHeight, 
		//		0, 0, (int)m_size.Width, (int)m_size.Height,
		//		m_imageData,
		//		&bmpInfo, DIB_RGB_COLORS, SRCCOPY) == 0)
		//	{
		//		BT_CORE_ERROR << "Failed to strech DIB bits. ::GetLastError() = " << std::endl;
		//	}

		//	// Render the scaled bitmap
		//	if (!::BitBlt(destDC, positionDestination.X, positionDestination.Y,
		//		adjustedWidth, adjustedHeight, m_attributes->m_hdc, 0, 0, SRCCOPY))
		//	{
		//		BT_CORE_ERROR << " - BitBlt ::GetLastError() = " << ::GetLastError() << std::endl;
		//	}
		//}
#endif
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect, bool isEnabled)
	{
	}
}