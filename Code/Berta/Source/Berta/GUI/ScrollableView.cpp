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

    void ScrollableView::SetViewRect(const Rectangle& clientAreaBounds)
    {
        if (m_clientAreaBounds == clientAreaBounds)
        {
            return;
        }
        m_clientAreaBounds = clientAreaBounds;
        CalculateViewport();
    }

    void ScrollableView::SetViewPadding(int top, int bottom, int left, int right)
    {
        m_viewPadding = { top, bottom, left, right };
    }

    void ScrollableView::SetScrollStep(int vertical, int horizontal)
    {
        m_scrollStep = { static_cast<uint32_t>(horizontal), static_cast<uint32_t>(vertical) };
    }

    void ScrollableView::SetOnScrollChange(OnScrollCallback callback)
    {
        m_onScroll = std::move(callback);
    }

    void ScrollableView::SetScrollBarVisibility(ScrollBarVisibility vertical, ScrollBarVisibility horizontal)
    {
        m_vVisibility = vertical;
        m_hVisibility = horizontal;
        CalculateViewport();
    }

    Rectangle ScrollableView::GetVisibleRect() const
    {
        return { m_scrollOffset.X, m_scrollOffset.Y, m_viewportRect.Width, m_viewportRect.Height };
    }

    void ScrollableView::CalculateViewport()
    {
        m_viewportRect = m_clientAreaBounds;
        auto scrollSize = static_cast<int>(m_owner->ToScale(m_owner->Appearance->ScrollBarSize));
        
        bool showV = (m_vVisibility == ScrollBarVisibility::Visible) || 
                  (m_vVisibility == ScrollBarVisibility::Auto && m_contentSize.Height > m_viewportRect.Height);
    
        if (showV)
        {
            m_viewportRect.Width = std::max<int>(0, static_cast<int>(m_viewportRect.Width) - scrollSize);
        }
        // 2. Evaluar necesidad visual en X
        bool showH = (m_hVisibility == ScrollBarVisibility::Visible) || 
                     (m_hVisibility == ScrollBarVisibility::Auto && m_contentSize.Width > m_viewportRect.Width);
    
        if (showH) 
        {
            m_viewportRect.Height = std::max<int>(0, static_cast<int>(m_viewportRect.Height) - scrollSize);
        
            // Dos-Pasadas: Si apareció la H, redujo la altura. ¿Necesitamos la V ahora?
            if (!showV && m_vVisibility == ScrollBarVisibility::Auto && m_contentSize.Height > m_viewportRect.Height) 
            {
                showV = true;
                m_viewportRect.Width = std::max<int>(0, static_cast<int>(m_viewportRect.Width) - scrollSize);
            }
        }

        UpdateScrollBars();
    }

    void ScrollableView::SetScrollToX(int offsetX)
    {
        int maxOffsetX = std::max<int>(0, m_contentSize.Width - m_viewportRect.Width);
        m_scrollOffset.X = std::clamp(offsetX, 0, maxOffsetX);
        if (m_scrollBarHoriz)
        {
            m_scrollBarHoriz->SetValue(m_scrollOffset.X);
        }
        NotifyChange();
    }

    void ScrollableView::SetScrollToY(int offsetY)
    {
        int maxOffsetY = std::max<int>(0, m_contentSize.Height - m_viewportRect.Height);
        m_scrollOffset.Y = std::clamp(offsetY, 0, maxOffsetY);
        if (m_scrollBarVert)
        {
            m_scrollBarVert->SetValue(m_scrollOffset.Y);
        }
        NotifyChange();
    }

    void ScrollableView::ResetScroll()
    {
        m_scrollOffset = {0, 0};
        CalculateViewport();
    }

    void ScrollableView::UpdateScrollBars()
    {
        bool needNotify = false;

        // --- 1. Lógica Visual de Scrollbars ---
        bool contentExceedsY = m_contentSize.Height > m_viewportRect.Height;
        bool shouldShowV = (m_vVisibility == ScrollBarVisibility::Visible) || 
                           (m_vVisibility == ScrollBarVisibility::Auto && contentExceedsY);

        if (!shouldShowV && m_scrollBarVert)
        {
            m_scrollBarVert.reset();
            needNotify = true;
        }
        else if (shouldShowV)
        {
            needNotify |= UpdateScrollBarInstance(true, contentExceedsY);
        }

        bool contentExceedsX = m_contentSize.Width > m_viewportRect.Width;
        bool shouldShowH = (m_hVisibility == ScrollBarVisibility::Visible) || 
                           (m_hVisibility == ScrollBarVisibility::Auto && contentExceedsX);

        if (!shouldShowH && m_scrollBarHoriz)
        {
            m_scrollBarHoriz.reset();
            needNotify = true;
        }
        else if (shouldShowH)
        {
            needNotify |= UpdateScrollBarInstance(false, contentExceedsX);
        }

        // --- 2. Abrazar la Realidad Lógica (Crucial para Scroll Hidden) ---
        // Incluso si las barras están Hidden, el límite de navegación matemática existe.
        int maxOffsetY = std::max<int>(0, (int)m_contentSize.Height - (int)m_viewportRect.Height);
        int maxOffsetX = std::max<int>(0, (int)m_contentSize.Width - (int)m_viewportRect.Width);
    
        int clampedY = std::clamp(m_scrollOffset.Y, 0, maxOffsetY);
        int clampedX = std::clamp(m_scrollOffset.X, 0, maxOffsetX);

        if (clampedY != m_scrollOffset.Y || clampedX != m_scrollOffset.X) 
        {
            m_scrollOffset = { clampedX, clampedY };
            needNotify = true;
        }

        if (needNotify)
        {
            NotifyChange();
        }
    }

    bool ScrollableView::UpdateScrollBarInstance(bool isVertical, bool contentExceeds)
    {
        bool changed = false;
        auto scrollSize = static_cast<int>(m_owner->ToScale(m_owner->Appearance->ScrollBarSize));
        
        Rectangle scrollRect;
        if (isVertical)
        {
            scrollRect = { m_clientAreaBounds.X + static_cast<int>(m_clientAreaBounds.Width) - scrollSize, 
                           m_clientAreaBounds.Y, (uint32_t)scrollSize, m_viewportRect.Height };
        }
        else
        {
            scrollRect = { m_clientAreaBounds.X, 
                           m_clientAreaBounds.Y + static_cast<int>(m_clientAreaBounds.Height) - scrollSize, 
                           m_viewportRect.Width, (uint32_t)scrollSize };
        }

        auto& barPtr = isVertical ? m_scrollBarVert : m_scrollBarHoriz;

        if (!barPtr)
        {
            barPtr = std::make_unique<ScrollBar>(m_owner, false, scrollRect, isVertical);
            barPtr->GetEvents().ValueChanged.Connect([this, isVertical](const ArgScrollBar& args)
            {
                if (isVertical) m_scrollOffset.Y = args.Value;
                else            m_scrollOffset.X = args.Value;
                
                NotifyChange();
            });
            changed = true;
        }
        else
        {
            GUI::MoveWindow(barPtr->Handle(), scrollRect);
        }

        // Si Visible está forzado pero no hay contenido, lo deshabilitamos (Estándar UI)
        //GUI::EnableWindow(barPtr->Handle(), contentExceeds);

        auto viewSize = isVertical ? m_viewportRect.Height : m_viewportRect.Width;
        auto contentLimit = isVertical ? m_contentSize.Height : m_contentSize.Width;
        auto stepSize = isVertical ? m_scrollStep.Height : m_scrollStep.Width;

        barPtr->SetMinMax(0, std::max<int>(0, (int)contentLimit - (int)viewSize));
        barPtr->SetPageStepValue((int)viewSize);
        barPtr->SetStepValue((int)stepSize);
    
        // Sincronizar el visualizador
        int currentOffset = isVertical ? m_scrollOffset.Y : m_scrollOffset.X;
        if (barPtr->GetValue() != currentOffset)
        {
            barPtr->SetValue(currentOffset);
        }

        return changed;
    }

    void ScrollableView::HandleMouseWheel(const ArgWheel& args)
    {
        bool targetIsVertical = args.IsVertical;
        if (targetIsVertical && !HasVerticalScroll() && HasHorizontalScroll())
        {
            targetIsVertical = false;
        }

        ScrollBar* activeBar = targetIsVertical ? m_scrollBarVert.get() : m_scrollBarHoriz.get();
        if (!activeBar)
        {
            return;
        }
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
        Point newScroll = m_scrollOffset;

        // Vertical
        if (m_contentSize.Height > m_viewportRect.Height)
        {
            int viewTop = m_scrollOffset.Y + m_viewPadding.Top;
            int viewBottom = m_scrollOffset.Y + static_cast<int>(m_viewportRect.Height) - m_viewPadding.Bottom;
            
            int targetTop = targetBounds.Y;
            int targetBottom = targetBounds.Y + static_cast<int>(targetBounds.Height);

            if (targetTop < viewTop) {
                newScroll.Y = targetTop - m_viewPadding.Top;
            }
            else if (targetBottom > viewBottom) {
                if (static_cast<int>(targetBounds.Height) > static_cast<int>(m_viewportRect.Height)) {
                    newScroll.Y = targetTop - m_viewPadding.Top;
                } else {
                    newScroll.Y = targetBottom - static_cast<int>(m_viewportRect.Height) + m_viewPadding.Bottom;
                }
            }
        }

        // Horizontal
        if (m_contentSize.Width > m_viewportRect.Width)
        {
            int viewLeft = m_scrollOffset.X + m_viewPadding.Left;
            int viewRight = m_scrollOffset.X + static_cast<int>(m_viewportRect.Width) - m_viewPadding.Right;

            int targetLeft = targetBounds.X;
            int targetRight = targetBounds.X + static_cast<int>(targetBounds.Width);

            if (targetLeft < viewLeft)
            {
                newScroll.X = targetLeft - m_viewPadding.Left;
            }
            else if (targetRight > viewRight)
            {
                if (static_cast<int>(targetBounds.Width) > static_cast<int>(m_viewportRect.Width))
                {
                    newScroll.X = targetLeft - m_viewPadding.Left;
                }
                else
                {
                    newScroll.X = targetRight - static_cast<int>(m_viewportRect.Width) + m_viewPadding.Right;
                }
            }
        }

        if (newScroll != m_scrollOffset)
        {
            SetScrollToX(newScroll.X);
            SetScrollToY(newScroll.Y);
            changed = true;
        }

        return changed;
    }

    void ScrollableView::NotifyChange() const
    {
        if (m_onScroll)
        {
            m_onScroll();
        }
    }
}
