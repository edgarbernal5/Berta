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
		void SetViewSize(const Size& size); // Tamaño del control contenedor
		void SetScrollStep(int vertical, int horizontal);
		
		void SetOnScrollChange(OnScrollCallback callback);
		
		void CalculateViewport();
		void HandleMouseWheel(const ArgWheel& args);
        
		// Auto-scroll para asegurar que un rectángulo en el espacio absoluto sea visible
		bool EnsureVisibility(const Rectangle& targetBounds); 

		// --- Getters para el Renderizado ---
        
		// Retorna el área útil donde el control puede dibujar (excluyendo scrollbars)
		Rectangle GetClientArea() const { return m_viewportRect; } 
        
		// EL MÉTODO CLAVE: Retorna {OffsetX, OffsetY, ClientWidth, ClientHeight}
		Rectangle GetVisibleRect() const; 
        
		Point GetScrollOffset() const { return m_scrollOffset; }
		
	private:
		void UpdateScrollBars();
		void UpdateVerticalScrollBar();
		void UpdateHorizontalScrollBar();
		void NotifyChange();
		
		Window* m_owner; 
		OnScrollCallback m_onScroll;

		Size m_contentSize{ 0, 0 };
		Size m_fullViewSize{ 0, 0 }; 
		Rectangle m_viewportRect{ 0, 0, 0, 0 }; 

		Point m_scrollOffset{ 0, 0 };
		Size m_scrollStep{ 20, 20 }; 

		bool m_needVerticalScroll{ false };
		bool m_needHorizontalScroll{ false };

		std::unique_ptr<ScrollBar> m_scrollBarVert;
		std::unique_ptr<ScrollBar> m_scrollBarHoriz;
	};
}

#endif