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
	//TODO: refactor Image class

	class AbstractImageAttributes
	{
	public:
		virtual ~AbstractImageAttributes() = default;

		virtual Size GetSize() const = 0;
		virtual void Open(const std::string& filepath) = 0;

		virtual void Paste(Graphics& destination, const Point& positionDestination) = 0;
		virtual void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) = 0;
	};

	class Image
	{
	public:
		Image() = default;
		Image(const std::string& filepath);
		Image(const Image& other);
		Image(Image&& other) noexcept;
		~Image();

		operator bool() const;

		Image& operator=(const Image& rhs);
		Image& operator=(Image&&) noexcept;

		Size GetSize() const { return m_attributes->GetSize(); }

		void Open(const std::string& filepath);
		void Paste(Graphics& destination, const Point& positionDestination);
		void Paste(Graphics& destination, const Rectangle& destinationRect);
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination);
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect);

	private:

		void CheckAndUpdateHdc(uint32_t currentDpi, bool enabled);
		void OpenIcon(const std::string& filepath);

//		//TODO: move this attributes to NativeAttributes
//		unsigned char* m_imageData{ nullptr };
//		Size m_size{};
//		int m_channels{ 0 };
//		bool m_hasTransparency{ false };
//		uint32_t m_lastDpi{ 0 };
//		bool m_enabled{ true };
//
//#if BT_PLATFORM_WINDOWS
//		bool m_isIcon{ false };
//#endif
		//TODO: create a polymorphic classes to handle all type of images (BMP, ICO, etc)
		std::shared_ptr<AbstractImageAttributes> m_attributes;
	};
}

#endif