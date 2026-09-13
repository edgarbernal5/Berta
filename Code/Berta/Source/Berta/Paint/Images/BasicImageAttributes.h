/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_IMAGE_ATTRIBUTES_HEADER
#define BT_BASIC_IMAGE_ATTRIBUTES_HEADER

#include <unordered_map>

#if BT_PLATFORM_WINDOWS
#include <wrl/client.h>
#endif

#include "Berta/Paint/Image.h"
#include "Berta/Paint/ColorBuffer.h"

namespace Berta
{
	class BasicImageAttributes : public AbstractImageAttributes
	{
	public:
		BasicImageAttributes();
		~BasicImageAttributes() override;

		Size GetSize() const override;
		void Open(const std::wstring& filepath) override;
		void OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels) override;
		
		void Paste(Graphics& destination, const Point& positionDestination) override;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) override;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination) override;
		
	private:
		void ReleaseNativeObjects();

#if BT_PLATFORM_WINDOWS
		std::unordered_map<HWND, Microsoft::WRL::ComPtr<ID2D1Bitmap>> m_bitmapCache;
#endif
		Size m_size{};
		int m_channels{ 0 };
		bool m_hasTransparency{ false };
	};
}

#endif