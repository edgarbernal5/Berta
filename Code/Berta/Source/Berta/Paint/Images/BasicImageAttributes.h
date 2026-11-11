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

#include <map>

#include "Berta/Paint/Image.h"
#include "Berta/Paint/ColorBuffer.h"

namespace Berta
{
	class BasicImageAttributes : public AbstractImageAttributes
	{
	public:
		BasicImageAttributes();
		~BasicImageAttributes() override;

		Size GetLogicalSize() const override;
		void Open(const std::wstring& filepath) override;
		void OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels) override;
		
		void AddVariant(float scaleFactor, const std::wstring& filepath) override;
		void AddVariantFromMemory(float scaleFactor, const uint8_t* pixels, uint32_t width, uint32_t height, int channels) override;
		
		void Paste(Graphics& destination, const Point& positionDestination) override;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) override;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination) override;
		
	private:
		std::pair<float, const ColorBuffer*> GetBestBuffer(float targetScale) const;
		void ReleaseNativeObjects();
		
#if BT_PLATFORM_WINDOWS
		struct BitmapCacheEntry
		{
			Microsoft::WRL::ComPtr<ID2D1Bitmap> Bitmap;
			float CachedScale{ 0.0f };
		};
		
		std::unordered_map<HWND, BitmapCacheEntry> m_bitmapCache;
#endif
		std::map<float, ColorBuffer> m_variants;
		Size m_logicalSize{};
		int m_channels{ 0 };
		bool m_hasTransparency{ false };
	};
}

#endif