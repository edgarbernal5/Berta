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
	enum class ScrollBarVisibility : uint8_t
	{
		Auto,
		Visible,
		Hidden
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
		void SetScrollBarVisibility(ScrollBarVisibility vertical, ScrollBarVisibility horizontal);
		
		void CalculateViewport();
		void HandleMouseWheel(const ArgWheel& args);
        
		bool EnsureVisibility(const Rectangle& targetBounds); 
		
		Rectangle GetClientArea() const { return m_viewportRect; } 
		Rectangle GetVisibleRect() const; 
		Point GetScrollOffset() const { return m_scrollOffset; }
		
		void SetScrollToX(int offsetX);
		void SetScrollToY(int offsetY);
		void ResetScroll();
		
		bool HasVerticalScroll() const { return m_scrollBarVert != nullptr; }
		bool HasHorizontalScroll() const  { return m_scrollBarHoriz != nullptr; }
		
	private:
		void UpdateScrollBars();
		bool UpdateScrollBarInstance(bool isVertical, bool contentExceeds);
		void NotifyChange() const;
		
		Window* m_owner; 
		OnScrollCallback m_onScroll;

		Size m_contentSize{ 0, 0 };
		Rectangle m_clientAreaBounds{ 0, 0, 0, 0 }; 
		Rectangle m_viewportRect{ 0, 0, 0, 0 }; 

		Padding m_viewPadding;
		
		Point m_scrollOffset{ 0, 0 };
		Size m_scrollStep{ 20, 20 }; 

		ScrollBarVisibility m_vVisibility{ ScrollBarVisibility::Auto };
		ScrollBarVisibility m_hVisibility{ ScrollBarVisibility::Auto };
		
		std::unique_ptr<ScrollBar> m_scrollBarVert;
		std::unique_ptr<ScrollBar> m_scrollBarHoriz;
	};
}

#endif