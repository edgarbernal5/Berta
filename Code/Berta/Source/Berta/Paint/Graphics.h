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
#include "Berta/API/PaintAPI.h"

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
		Graphics(const Size& size, uint32_t dpi, API::RootPaintNativeHandle rootPaintHandle);
		//Graphics(const Graphics& other);
		Graphics(Graphics&& other) noexcept;
		~Graphics(); 
		
		Graphics& operator=(const Graphics& other);
		Graphics& operator=(Graphics&& other);

		enum class ArrowDirection
		{
			Downwards,
			Upwards,
			Left,
			Right
		};

		enum class LineStyle
		{
			Solid,
			Dash,
			Dotted
		};

		void Build(const Size& size, API::RootPaintNativeHandle rootPaintHandle);
		void BuildFont(uint32_t dpi);
		void Rebuild(const Size& size, API::RootPaintNativeHandle rootPaintHandle);
		void Blend(const Rectangle& blendDestRectangle, const Graphics& graphicsSource, const Point& pointSource, double alpha);
		void BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource);
		
		void DrawLine(const Point& point1, const Point& point2, const Color& color, LineStyle style = LineStyle::Solid);
		void DrawLine(const Point& point1, const Point& point2, float strokeWidth, const Color& color, LineStyle style = LineStyle::Solid);
		void DrawBeginLine(const Point& point, const Color& color, LineStyle style = LineStyle::Solid);
		void DrawLineTo(const Point& point, const Color& color);

		void DrawRectangle(const Color& color, bool solid, float strokeWidth = 1.0f);
		void DrawRectangle(const Rectangle& rectangle, const Color& color, bool solid, float strokeWidth = 1.0f);
		void DrawRectangle(const Rectangle& rectangle, const Color& borderColor, bool solid, const Color& solidColor, float strokeWidth = 1.0f);
		void DrawString(const Point& position, const std::wstring& str, const Color& color);
		void DrawString(const Point& position, const std::string& str, const Color& color);

		void DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor);
		void DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor, bool solid, const Color& solidColor = {}, float strokeWidth = 1.0f);
		void DrawRoundRectBox(const Rectangle& rect, const Color& color, const Color& bordercolor, bool solid);
		void DrawRoundRectBox(const Rectangle& rect, int radius, const Color& color, const Color& bordercolor, bool solid);
		void DrawGradientFill(const Rectangle& rect, const Color& startColor, const Color& endColor);
		void DrawCircle(const Point& dest, int radius, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth = 1.0f);
		void DrawEllipse(const Rectangle& rect, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth = 1.0f);

		uint32_t GetDpi() const { return m_dpi; }
		const Size& GetSize() const { return m_size; }
		const Size& GetTextExtent() const 
		{
			return m_attributes->m_textExtent;
		}
		Size GetTextExtent(const std::wstring& str);
		Size GetTextExtent(const std::string& str);
		Size GetTextExtent(const std::wstring& str, size_t length);

		PaintNativeHandle* GetHandle() const { return m_attributes.get(); }

		void Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const;
		void Paste(API::RootPaintNativeHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const;
		void Paste(API::NativeWindowHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const;
		void Paste(API::RootPaintNativeHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const;

		void Begin();
		void Flush();

		void Swap(Graphics& other);
		void Release();
		bool IsEnabledAliasing();
		void EnabledAliasing(bool enabled);

		bool IsValid() const
		{
#ifdef BT_PLATFORM_WINDOWS
			return m_attributes != nullptr && m_attributes->m_bitmapRT;
#else
			return m_attributes != nullptr;
#endif
		}
	private:

		uint32_t m_dpi{ 96u };
		Size m_size{};
		API::RootPaintNativeHandle m_rootPaintNativeHandle;
		std::unique_ptr<PaintNativeHandle> m_attributes;
	};
}

#endif
