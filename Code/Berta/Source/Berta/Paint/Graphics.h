/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_GRAPHICS_HEADER
#define BT_GRAPHICS_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/API/WindowAPI.h"

namespace Berta
{
	/*
	* Wrapper for GDI functions.
	*/
	class Graphics
	{
	public:
		Graphics();
		Graphics(const Size& size);

		void Build(const Size& size);
		void BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource);
		void DrawRectangle(const Rectangle& rectangle, bool solid);
		void DrawRectangle(const Color& color, bool solid);
		void DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid);

		void Paste(API::NativeWindowHandle destination, const Rectangle& areaToUpdate, int x, int y) const;
		void Paste(API::NativeWindowHandle destination, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const;
		void Flush();
	private:
		Size m_size;
		HDC m_hdc{ nullptr };
		HBITMAP	m_hBitmap{ nullptr };
	};
}

#endif