/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_IMAGE_HEADER
#define BT_IMAGE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Base.h"

namespace Berta
{
	class Graphics;

	class Image
	{
	public:
		Image() = default;
		Image(const std::string& filepath);
		~Image();

		operator bool() const;

		Image& operator=(const Image& rhs);
		Image& operator=(Image&&) noexcept;

		Size GetSize() const { return m_size; }

		void Open(const std::string& filepath);
		void Paste(Graphics& destination, const Point& positionDestination, bool isEnabled = true);
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination, bool isEnabled);

	private:
		struct NativeAttributes
		{
			~NativeAttributes();

#if BT_PLATFORM_WINDOWS
			HDC m_hdc{ nullptr };
			HBITMAP m_hBitmap{ nullptr };
			HICON m_hIcon{ nullptr };
#endif
		};
		void CheckAndUpdateHdc(uint32_t currentDpi, bool enabled);
		void OpenIcon(const std::string& filepath);

		unsigned char* m_imageData{ nullptr };
		Size m_size{};
		int m_channels{ 0 };
		bool m_hasTransparency{ false };
		uint32_t m_lastDpi{ 0 };
		bool m_enabled{ true };

#if BT_PLATFORM_WINDOWS
		bool m_isIcon{ false };
#endif
		std::shared_ptr<NativeAttributes> m_attributes;
	};
}

#endif