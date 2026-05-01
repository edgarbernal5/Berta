/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_GRAPHICS_HEADER
#define BT_GRAPHICS_HEADER

#include <memory>
#include <stack>
#include <string_view>

#include "Berta/Core/BasicTypes.h"
#include "Berta/API/WindowAPI.h"
#include "Berta/API/PaintAPI.h"

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Platform/Windows/ResourceCache.h"
#endif

namespace Berta
{
	struct TextFormatOptions
	{
		bool WordWrap{ false };
		HorizontalAlign AlignX { HorizontalAlign::Left };
		VerticalAlign AlignY { VerticalAlign::Top };
	};
	
	class Graphics
	{
	public:
		friend class Image;
		friend class BasicImageAttributes;

	public:
		Graphics();
		Graphics(const Size& size, uint32_t dpi, API::RootPaintNativeHandle rootPaintHandle);
		explicit Graphics(API::RootPaintNativeHandle rootPaintHandle);

		Graphics(const Graphics& other) = delete;
		Graphics(Graphics&& other) noexcept;
		~Graphics(); 
		
		Graphics& operator=(const Graphics& other) = delete;
		Graphics& operator=(Graphics&& other) noexcept;

		enum class ArrowDirection : uint8_t
		{
			Downwards,
			Upwards,
			Left,
			Right
		};

		void Build(API::RootPaintNativeHandle rootPaintHandle);
		void Build(const Size& size, API::RootPaintNativeHandle rootPaintHandle);
		void BuildFont(uint32_t dpi);
		void Rebuild(API::RootPaintNativeHandle rootPaintHandle);
		void Rebuild(const Size& size, API::RootPaintNativeHandle rootPaintHandle);
		
		void CreateTextLayout(const wchar_t* wstr, UINT32 length, uint32_t width, uint32_t height);
		
		void Blend(const Rectangle& blendDestRectangle, const Graphics& graphicsSource, const Point& pointSource, double alpha);
		void BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource);
		
		void DrawLine(const Point& point1, const Point& point2, const Color& color, LineStyle style = LineStyle::Solid);
		void DrawLine(const Point& point1, const Point& point2, float strokeWidth, const Color& color, LineStyle style = LineStyle::Solid);
		
		void DrawRectangle(const Rectangle& rect, const Color& borderColor, float strokeWidth = 1.0f);
		void FillRectangle(const Rectangle& rect, const Color& fillColor);
		void FillAndDrawRectangle(const Rectangle& rect, const Color& solidColor, const Color& borderColor, float strokeWidth = 1.0f);
		
		void DrawTopRoundedRectangle(const Rectangle& rect, float radius, const Color& borderColor, bool closeFigure, float strokeWidth = 1.0f);
		void FillTopRoundedRectangle(const Rectangle& rect, float radius, const Color& fillColor);
		
		void DrawBottomRoundedRectangle(const Rectangle& rect, float radius, Color borderColor, bool closeFigure, float strokeWidth = 1.0f);
		void FillBottomRoundedRectangle(const Rectangle& rect, float radius, Color fillColor);
		
		void DrawString(const Point& position, std::string_view strView, const Color& color);
		void DrawString(const Point& position, std::wstring_view wstrView, const Color& color);
		
		void DrawString(const Rectangle& area, std::string_view strView, const Color& color, const TextFormatOptions& options = {});
		void DrawString(const Rectangle& area, std::wstring_view wstrView, const Color& color, const TextFormatOptions& options = {});
		
		void DrawTextLayout(const TextPaintNativeHandle& handle, const Point& origin, const Color& color);
		
		void DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor);
		void DrawArrow(const Rectangle& rect, int arrowLength, int arrowWidth, ArrowDirection direction, const Color& borderColor, bool solid, const Color& solidColor = {}, float strokeWidth = 1.0f);
		void DrawRoundRectBox(const Rectangle& rect, const Color& color, const Color& bordercolor, bool solid);
		void DrawRoundRectBox(const Rectangle& rect, int radius, const Color& color, const Color& bordercolor, bool solid);
		void DrawGradientFill(const Rectangle& rect, const Color& startColor, const Color& endColor);
		void DrawCircle(const Point& dest, int radius, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth = 1.0f);
		void DrawEllipse(const Rectangle& dest, const Color& fillColor, const Color& borderColor, bool solid, float strokeWidth = 1.0f);

		uint32_t GetDpi() const { return m_dpi; }
		const Size& GetSize() const { return m_size; }
		
		const Size& GetTextExtent() const { return m_attributes->m_textExtent; }
		Size GetTextExtent(const std::wstring& wstr) const;
		Size GetTextExtent(const std::string& str) const;
		Size GetTextExtent(const std::wstring& wstr, size_t length) const;
		Size GetTextExtent(const std::wstring& wstr, const Rectangle& area) const;
		Size GetTextExtent(const std::string& str, const Rectangle& area) const;
		Size GetTextExtent(std::wstring_view wstr) const;
		
		uint32_t GetCaretHeight() const;

		const API::RootPaintNativeHandle* GetHandle() const { return &m_rootPaintNativeHandle; }

		void Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const;
		void Paste(API::RootPaintNativeHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const;
		void Paste(API::NativeWindowHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const;
		void Paste(API::RootPaintNativeHandle destinationHandle, int dx, int dy, uint32_t width, uint32_t height, int sx, int sy) const;

		void Begin();
		void Flush();

		void SetTransform(const Rectangle& area);
		void SetClipping(const Rectangle& area) const;
		void EndClipping();
		
		void PushTranslation(int x, int y);
		void PopTranslation();

		void Swap(Graphics& other);
		void Release();
		
		bool IsEnabledAliasing();
		void EnabledAliasing(bool enabled);

		bool IsValid() const
		{
#ifdef BT_PLATFORM_WINDOWS
			return m_targetRT;
#else
			return m_attributes != nullptr;
#endif
		}
		
		PaintNativeHandle* GetNativeHandle() const
		{
			if (!IsValid())
			{
				return nullptr;
			}
			
			return m_attributes.get();
		}
	private:
		uint32_t m_dpi{ 96u };
		Size m_size{};
		API::RootPaintNativeHandle m_rootPaintNativeHandle;
		std::unique_ptr<PaintNativeHandle> m_attributes;

#ifdef BT_PLATFORM_WINDOWS
		ID2D1PathGeometry* CreateTopRoundedGeometry(const Rectangle& rect, float radius, bool closeFigure) const;
		ID2D1PathGeometry* CreateBottomRoundedGeometry(const Rectangle& rect, float radius, bool closeFigure) const;
		
		std::stack<D2D1_MATRIX_3X2_F> m_transformStack;
		
		ID2D1RenderTarget* m_targetRT{ nullptr };
		
		ResourceCache m_resourceCache;
#endif
	};
}

#endif
