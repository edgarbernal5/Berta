/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "IconImageAttributes.h"

namespace Berta
{
	/*
	#if BT_PLATFORM_WINDOWS
		std::filesystem::path path{ filepath };
		if (path.has_extension() && path.extension() == ".ico")
		{
			OpenIcon(filepath);
			return;
		}
		m_isIcon = false;
#endif*/

	/*
	
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

		m_attributes.reset(new PaintNativeHandle());
		m_attributes->m_hIcon = hIcon;
		*/


	/*
	Draw()

	*/


	IconImageAttributes::IconImageAttributes()
	{
	}

	IconImageAttributes::~IconImageAttributes()
	{
#if BT_PLATFORM_WINDOWS
		if (m_hIcon)
		{
			::DestroyIcon(m_hIcon);
			m_hIcon = nullptr;
		}
#endif
	}

	Size IconImageAttributes::GetSize() const
	{
		return Size();
	}

	void IconImageAttributes::Open(const std::string& filepath)
	{
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
		m_hIcon = hIcon;
	}

	void IconImageAttributes::Paste(Graphics& destination, const Point& positionDestination)
	{
		/*


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

		*/
	}

	void IconImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect)
	{
	}

}