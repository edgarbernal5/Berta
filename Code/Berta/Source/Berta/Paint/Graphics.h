/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_GRAPHICS_HEADER
#define BT_GRAPHICS_HEADER

#include <memory>
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
		friend class Image;
		friend class BasicImageAttributes;

	public:
		Graphics();
		Graphics(const Size& size);
		Graphics(const Graphics& other);
		Graphics(Graphics&& other) noexcept;
		~Graphics();

		enum class ArrowDirection
		{
			Downwards,
			Upwards,
			Left,
			Right
		};

		void Build(const Size& size);
		void BuildFont(uint32_t dpi);
		void Blend(const Graphics& graphicsSource, const Point& pointSource);
		void BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource);
		
		void DrawLine(const Point& point1, const Point& point2, const Color& color);
		void DrawBeginLine(const Point& point, const Color& color);
		void DrawLineTo(const Point& point, const Color& color);

		void DrawRectangle(const Color& color, bool solid);
		void DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid);
		void DrawString(const Point& position, const std::wstring& str, const Color& color);
		void DrawString(const Point& position, const std::string& str, const Color& color);

		void DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, const Color& color, ArrowDirection direction = ArrowDirection::Downwards, bool solid = true);
		void DrawRoundRectBox(const Rectangle& rect, const Color& color, bool solid);
		void DrawRoundRectBox(const Rectangle& rect, int radius, const Color& color, bool solid);

		uint32_t GetDpi() const { return m_dpi; }
		const Size& GetSize() const { return m_attributes->m_size; }
		const Size& GetTextExtent() const { return m_attributes->m_textExtent; }
		Size GetTextExtent(const std::wstring& str);
		Size GetTextExtent(const std::string& str);
		Size GetTextExtent(const std::wstring& str, size_t length);

		void Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const;
		void Paste(API::NativeWindowHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const;
		void Flush();

		void Swap(Graphics& other);
		void Release();
	private:
#ifdef BT_PLATFORM_WINDOWS
		struct NativeAttributes
		{
			HDC m_hdc{ nullptr };
			HBITMAP	m_hBitmap{ nullptr };
			HFONT m_hFont{ nullptr };
			uint32_t m_lastForegroundColor{ 0 };
			Size m_size;
			Size m_textExtent;

			NativeAttributes() = default;
			~NativeAttributes();

			NativeAttributes(const NativeAttributes&) = delete;
			NativeAttributes& operator=(const NativeAttributes&) = delete;
		};
#else
		struct NativeAttributes
		{
			Size m_size;

			NativeAttributes() = default;
			~NativeAttributes();
		};
#endif
		uint32_t m_dpi{ 96u };
		std::unique_ptr<NativeAttributes> m_attributes;
	};
}

#endif