/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SCROLL_MANAGER_HEADER
#define BT_SCROLL_MANAGER_HEADER

#include "Berta/Controls/ScrollBar.h"
#include "Berta/GUI/Control.h"

namespace Berta
{
	struct ScrollMetrics
	{
		Size contentSize;
		Size viewportSize;
		Point currentOffset;
	};
	
	class IScrollable
	{
	public:
		virtual ~IScrollable() = default;
		virtual ScrollMetrics GetScrollMetrics() const = 0;
		virtual void OnScrollCallback(const Point& newOffset) = 0;
	};
	
	class ScrollableView
	{
	public:
		using OnScrollCallback = std::function<void()>;
		
	public:
		ScrollableView(Window* owner);
		~ScrollableView() = default;
		
		void SetContentSize(const Size& size);
		void SetViewRect(const Rectangle& clientAreaBounds);
		void SetViewPadding(int top, int bottom = 0, int left = 0, int right = 0);
		
		void SetScrollStep(int vertical, int horizontal);
		void SetOnScrollChange(OnScrollCallback callback);
		
		void CalculateViewport();
		void HandleMouseWheel(const ArgWheel& args);
        
		bool EnsureVisibility(const Rectangle& targetBounds) const; 
		
		Rectangle GetClientArea() const { return m_viewportRect; } 
		Rectangle GetVisibleRect() const; 
		Point GetScrollOffset() const { return m_scrollOffset; }
		
		void SetScrollToX(int offsetX);
		void SetScrollToY(int offsetY);
		
		bool HasVerticalScroll() const { return m_scrollBarVert != nullptr; }
		bool HasHorizontalScroll() const  { return m_scrollBarHoriz != nullptr; }
	private:
		void UpdateScrollBars();
		void UpdateVerticalScrollBar();
		void UpdateHorizontalScrollBar();
		void NotifyChange() const;
		
		Window* m_owner; 
		OnScrollCallback m_onScroll;

		Size m_contentSize{ 0, 0 };
		Rectangle m_clientAreaBounds{ 0, 0, 0, 0 }; 
		Rectangle m_viewportRect{ 0, 0, 0, 0 }; 

		Padding m_viewPadding;
		
		Point m_scrollOffset{ 0, 0 };
		Size m_scrollStep{ 20, 20 }; 

		bool m_needVerticalScroll{ false };
		bool m_needHorizontalScroll{ false };

		std::unique_ptr<ScrollBar> m_scrollBarVert;
		std::unique_ptr<ScrollBar> m_scrollBarHoriz;
	};
}

#endif