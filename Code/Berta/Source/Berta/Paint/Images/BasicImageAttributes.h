/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BASIC_IMAGE_ATTRIBUTES_HEADER
#define BT_BASIC_IMAGE_ATTRIBUTES_HEADER

#include "Berta/Paint/Image.h"
#include "Berta/Paint/ColorBuffer.h"

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
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) override;

	private:
		void ReleaseNativeObjects();

		Size m_size{};
		int m_channels{ 0 };
		bool m_hasTransparency{ false };
	};
}

#endif