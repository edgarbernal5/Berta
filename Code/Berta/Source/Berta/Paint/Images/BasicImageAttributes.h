/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_IMAGE_ATTRIBUTES_HEADER
#define BT_BASIC_IMAGE_ATTRIBUTES_HEADER

#include "Berta/Paint/Image.h"

namespace Berta
{
	class BasicImageAttributes : public AbstractImageAttributes
	{
	public:
		BasicImageAttributes();
		~BasicImageAttributes();

		Size GetSize() const override;
		void Open(const std::string& filepath) override;
		void Paste(Graphics& destination, const Point& positionDestination) override;

	private:
		//TODO: move this attributes to NativeAttributes
		unsigned char* m_imageData{ nullptr };
		Size m_size{};
		int m_channels{ 0 };
		bool m_hasTransparency{ false };
		uint32_t m_lastDpi{ 0 };
		bool m_enabled{ true };


#if BT_PLATFORM_WINDOWS
		HDC m_hdc{ nullptr };
		HBITMAP m_hBitmap{ nullptr };
#endif
	};
}

#endif