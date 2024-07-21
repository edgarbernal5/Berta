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

		void Open(const std::string& filepath);
		void Paste(Graphics& destination, const Point& positionDestination);
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination);

	private:
		struct NativeAttributes
		{
			Size m_size{};
			int m_channels{0};
			HDC m_hdc{ nullptr };
			HBITMAP hBitmap{ nullptr };

		};
		unsigned char* m_imageData{ nullptr };
		std::unique_ptr<NativeAttributes> m_attributes;
	};
}

#endif