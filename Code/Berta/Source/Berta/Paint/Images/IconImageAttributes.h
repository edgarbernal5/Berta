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
		~IconImageAttributes();

		Size GetSize() const override;
		void Open(const std::string& filepath) override;
		void Paste(Graphics& destination, const Point& positionDestination) override;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) override;

	private:
#if BT_PLATFORM_WINDOWS
		HICON m_hIcon{ nullptr };
#endif
	};
}

#endif