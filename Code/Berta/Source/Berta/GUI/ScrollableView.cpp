/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ScrollableView.h"

namespace Berta
{
    ScrollableView::ScrollableView(Window* ownerWindow)
        : m_owner(ownerWindow)
    {
    }

    void ScrollableView::SetContentSize(const Size& size)
    {
        if (m_contentSize == size)
        {
            return;
        }
        m_contentSize = size;
        CalculateViewport();
    }

    void ScrollableView::SetViewSize(const Size& size)
    {
        if (m_fullViewSize == size)
        {
            return;
        }
        m_fullViewSize = size;
        CalculateViewport();
    }

    void ScrollableView::SetScrollStep(int vertical, int horizontal)
    {
        m_scrollStep = { static_cast<uint32_t>(horizontal), static_cast<uint32_t>(vertical) };
    }

    void ScrollableView::SetOnScrollChange(OnScrollCallback callback)
    {
        m_onScroll = std::move(callback);
    }

    Rectangle ScrollableView::GetVisibleRect() const
    {
        return { m_scrollOffset.X, m_scrollOffset.Y, m_viewportRect.Width, m_viewportRect.Height };
    }

    void ScrollableView::CalculateViewport()
    {
        m_viewportRect = m_fullViewSize.ToRectangle();
        
        // Ajustes de borde estándar
        m_viewportRect.X = m_viewportRect.Y = 1;
        m_viewportRect.Width = (std::max)(0, static_cast<int>(m_viewportRect.Width) - 2);
        m_viewportRect.Height = (std::max)(0, static_cast<int>(m_viewportRect.Height) - 2);

        auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);

        // Lógica de necesidad de scroll cruzado
        m_needVerticalScroll = m_contentSize.Height > m_viewportRect.Height;
        if (m_needVerticalScroll)
        {
            m_viewportRect.Width = (std::max)(0, static_cast<int>(m_viewportRect.Width) - static_cast<int>(scrollSize));
        }

        m_needHorizontalScroll = m_contentSize.Width > m_viewportRect.Width;
        if (m_needHorizontalScroll)
        {
            m_viewportRect.Height = (std::max)(0, static_cast<int>(m_viewportRect.Height) - static_cast<int>(scrollSize));
            
            if (!m_needVerticalScroll)
            {
                m_needVerticalScroll = m_contentSize.Height > m_viewportRect.Height;
                if (m_needVerticalScroll)
                {
                    m_viewportRect.Width = std::max<int>(0, static_cast<int>(m_viewportRect.Width) - static_cast<int>(scrollSize));
                }
            }
        }
        
        UpdateScrollBars();
    }

    void ScrollableView::UpdateScrollBars()
    {
        if (!m_needVerticalScroll && m_scrollBarVert)
        {
            m_scrollBarVert.reset();
            m_scrollOffset.Y = 0;
            NotifyChange();
        }
        else if (m_needVerticalScroll)
        {
            UpdateVerticalScrollBar();
        }

        if (!m_needHorizontalScroll && m_scrollBarHoriz)
        {
            m_scrollBarHoriz.reset();
            m_scrollOffset.X = 0;
            NotifyChange();
        }
        else if (m_needHorizontalScroll)
        {
            UpdateHorizontalScrollBar();
        }
    }

    void ScrollableView::UpdateVerticalScrollBar()
    {
        auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
        Rectangle scrollRect{
            static_cast<int>(m_fullViewSize.Width - scrollSize) - 1, 
            1, 
            scrollSize, 
            m_fullViewSize.Height - 2u 
        };

        if (m_needHorizontalScroll) scrollRect.Height -= scrollSize;

        if (!m_scrollBarVert)
        {
            m_scrollBarVert = std::make_unique<ScrollBar>(m_owner, false, scrollRect);
            m_scrollBarVert->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
            {
                m_scrollOffset.Y = args.Value;
                NotifyChange();
            });
        }
        else
        {
            GUI::MoveWindow(m_scrollBarVert->Handle(), scrollRect);
        }

        m_scrollBarVert->SetMinMax(0, static_cast<int>(m_contentSize.Height - m_viewportRect.Height));
        m_scrollBarVert->SetPageStepValue(m_viewportRect.Height);
        m_scrollBarVert->SetStepValue(m_scrollStep.Height);
        
        // Auto-corrección si el contenido se redujo
        int clampedVal = std::clamp(m_scrollOffset.Y, 0, m_scrollBarVert->GetMax());
        if (clampedVal != m_scrollOffset.Y)
        {
            m_scrollBarVert->SetValue(clampedVal);
        }
    }

    void ScrollableView::UpdateHorizontalScrollBar()
    {

    }

    void ScrollableView::HandleMouseWheel(const ArgWheel& args)
    {
        ScrollBar* activeBar = args.IsVertical ? m_scrollBarVert.get() : m_scrollBarHoriz.get();
        if (!activeBar) return;

        int direction = (args.WheelDelta > 0 ? -1 : 1) * activeBar->GetStepValue();
        int currentVal = args.IsVertical ? m_scrollOffset.Y : m_scrollOffset.X;
        
        int newVal = std::clamp(currentVal + direction, activeBar->GetMin(), activeBar->GetMax());

        if (newVal != currentVal)
        {
            activeBar->SetValue(newVal); 
            GUI::MarkAsNeedUpdate(activeBar->Handle());
        }
    }

    bool ScrollableView::EnsureVisibility(const Rectangle& targetBounds)
    {
        bool changed = false;

        // Eje Y
        if (m_scrollBarVert)
        {
            int viewTop = m_scrollOffset.Y;
            int viewBottom = m_scrollOffset.Y + static_cast<int>(m_viewportRect.Height);
            int newY = m_scrollOffset.Y;

            if (targetBounds.Y < viewTop)
            {
                newY = targetBounds.Y;
            }
            else if (targetBounds.Y + targetBounds.Height > viewBottom)
            {
                newY = targetBounds.Y + static_cast<int>(targetBounds.Height) - static_cast<int>(m_viewportRect.Height);
            }
                
            if (newY != m_scrollOffset.Y)
            {
                m_scrollBarVert->SetValue(newY);
                changed = true;
            }
        }

        // Eje X (Aplica la misma lógica para el ancho si es necesario)
        // ...

        return changed;
    }

    void ScrollableView::NotifyChange()
    {
        if (m_onScroll)
        {
            m_onScroll();
        }
    }
}
