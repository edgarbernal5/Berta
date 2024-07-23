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
#if BT_PLATFORM_WINDOWS
			if (m_attributes->m_hBitmap)
			{
				::DeleteObject(m_attributes->m_hBitmap);
				m_attributes->m_hBitmap = nullptr;
			}
			if (m_attributes->m_hdc)
			{
				::DeleteDC(m_attributes->m_hdc);
				m_attributes->m_hdc = nullptr;
			}
			if (m_attributes->m_hIcon)
			{
				::DestroyIcon(m_attributes->m_hIcon);
				m_attributes->m_hIcon = nullptr;
			}
#endif
			m_attributes.reset();
		}
	}

	Image::operator bool() const
	{
		return m_attributes.get() != nullptr;
	}

	Image& Image::operator=(const Image& rhs)
	{
		if (this != &rhs)
		{
			m_attributes = rhs.m_attributes;
			m_size = rhs.m_size;
			m_channels = rhs.m_channels;
			m_hasTransparency = rhs.m_hasTransparency;
			m_imageData = rhs.m_imageData;
			m_isIcon = rhs.m_isIcon;
		}

		return *this;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		if (this != &other)
		{
			m_attributes = std::move(other.m_attributes);
			m_size = std::move(other.m_size);
			m_channels = std::move(other.m_channels);
			m_hasTransparency = std::move(other.m_hasTransparency);
			m_imageData = std::move(other.m_imageData);
			m_isIcon = std::move(other.m_isIcon);
		}

		return *this;
	}

	void Image::Open(const std::string& filepath)
	{
#if BT_PLATFORM_WINDOWS
		std::filesystem::path path{ filepath };
		if (path.has_extension() && path.extension() == ".ico")
		{
			OpenIcon(filepath);
			return;
		}
		m_isIcon = false;
#endif

		int width, height, channels;
		unsigned char* imageData = stbi_load(filepath.c_str(), &width, &height, &channels, 0); // Don't force RGBA
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

	void Image::OpenIcon(const std::string& filepath)
	{
		m_isIcon = true;

		std::filesystem::path path{ filepath };
		auto hIcon = (HICON)LoadImage
		(
			NULL,                 // HINSTANCE: Handle to the instance of the module that contains the image
			path.c_str(),     // Image file path
			IMAGE_ICON,           // Image type: IMAGE_ICON
			0,                    // Desired width (0 to use actual width)
			0,                    // Desired height (0 to use actual height)
			LR_LOADFROMFILE | LR_DEFAULTSIZE // Load flags
		);

		if (!hIcon)
		{
			BT_CORE_ERROR << "Failed to load icon: " << GetLastError() << std::endl;
		}

		m_attributes.reset(new NativeAttributes());
		m_attributes->m_hIcon = hIcon;
	}

	void Image::CheckAndUpdateHdc(uint32_t currentDpi)
	{
		if (m_lastDpi == currentDpi)
		{
			return;
		}
		float scaleFactor = m_hasTransparency ? 1.0f : currentDpi / 96.0f;

		int adjustedWidth = static_cast<int>(m_size.Width * scaleFactor);
		int adjustedHeight = static_cast<int>(m_size.Height * scaleFactor);

		HDC hdc = ::GetDC(NULL);
		// Create compatible DC
		HDC hdcMem = ::CreateCompatibleDC(hdc);

		::BITMAPINFO bmpInfo;
		ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = static_cast<LONG>(adjustedWidth);
		bmpInfo.bmiHeader.biHeight = -static_cast<LONG>(adjustedHeight);  // top-down image
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
			//TODO: DONT KNOW WHY I HAVE TO CALL THIS!
			memcpy(pBits, m_imageData, m_size.Width * m_size.Height * 4);
		}

		m_attributes->m_hBitmap = hBitmap;
		m_attributes->m_hdc = hdcMem;

		m_lastDpi = currentDpi;
		::ReleaseDC(0, hdc);
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination)
	{
		Paste(Rectangle{ m_size }, destination, positionDestination);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination)
	{
		if (!m_attributes)
		{
			return;
		}

#if BT_PLATFORM_WINDOWS
		HDC& destDC = destination.m_attributes->m_hdc;

		CheckAndUpdateHdc(destination.GetDpi());
		
		if (m_isIcon)
		{
			::DrawIconEx
			(
				destDC,					// HDC: Handle to device context
				positionDestination.X,	// X-coordinate of the upper-left corner
				positionDestination.Y,	// Y-coordinate of the upper-left corner
				m_attributes->m_hIcon,	// HICON: Handle to the icon to draw
				sourceRect.Width,		// Width of the icon
				sourceRect.Height,		// Height of the icon
				0,						// Frame index for animated icons (0 for single icons)
				NULL,					// Handle to a background brush (NULL if no background)
				DI_NORMAL				// Drawing flags
			);

			return;
		}

		HBITMAP hOldBitmap = (HBITMAP)SelectObject(m_attributes->m_hdc, m_attributes->m_hBitmap);
		if (m_hasTransparency)
		{
			// Set up alpha blending
			BLENDFUNCTION blendFunc;
			blendFunc.BlendOp = AC_SRC_OVER;
			blendFunc.BlendFlags = 0;
			blendFunc.SourceConstantAlpha = 255;
			blendFunc.AlphaFormat = AC_SRC_ALPHA;

			float scaleFactor = destination.GetDpi() / 96.0f;

			// Apply alpha blending
			if (!::AlphaBlend(destDC, positionDestination.X, positionDestination.Y,
				(int)(m_size.Width * scaleFactor), (int)(m_size.Height * scaleFactor), m_attributes->m_hdc,
				sourceRect.X, sourceRect.Y,
				m_size.Width, m_size.Height, blendFunc))
			{
				BT_CORE_ERROR << "Error en AlphaBlend. " << GetLastError() << std::endl;
			}
		}
		else
		{
			float scaleFactor = destination.GetDpi() / 96.0f;

			int adjustedWidth = static_cast<int>(m_size.Width * scaleFactor);
			int adjustedHeight = static_cast<int>(m_size.Height * scaleFactor);

			::BITMAPINFO bmpInfo;
			ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmpInfo.bmiHeader.biWidth = static_cast<LONG>(m_size.Width);
			bmpInfo.bmiHeader.biHeight = -static_cast<LONG>(m_size.Height);
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = m_channels * 8;
			bmpInfo.bmiHeader.biCompression = BI_RGB;

			// Stretch the image data to the scaled bitmap
			if (::StretchDIBits(m_attributes->m_hdc, 0, 0, adjustedWidth, adjustedHeight, 
				0, 0, (int)m_size.Width, (int)m_size.Height,
				m_imageData,
				&bmpInfo, DIB_RGB_COLORS, SRCCOPY) == 0)
			{
				BT_CORE_ERROR << "Failed to strech DIB bits." << std::endl;
			}

			// Render the scaled bitmap
			if (!::BitBlt(destDC, positionDestination.X, positionDestination.Y,
				adjustedWidth, adjustedHeight, m_attributes->m_hdc, 0, 0, SRCCOPY))
			{
				BT_CORE_ERROR << " - BitBlt ::GetLastError() = " << ::GetLastError() << std::endl;
			}

		}
#endif
	}
}