/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_ICON_ATTRIBUTES_HEADER
#define BT_ICON_ATTRIBUTES_HEADER

#include "Berta/Paint/Image.h"

namespace Berta
{
	class IconImageAttributes : public AbstractImageAttributes
	{
	public:
		IconImageAttributes();
		~IconImageAttributes() override;

		Size GetSize() const override;
		void Open(const std::wstring& filepath) override;
		void OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels) override;
		
		void Paste(Graphics& destination, const Point& positionDestination) override;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) override;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination) override;
		
	private:
#if BT_PLATFORM_WINDOWS
		HICON m_hIcon{ nullptr };
#endif
	};
}

#endif