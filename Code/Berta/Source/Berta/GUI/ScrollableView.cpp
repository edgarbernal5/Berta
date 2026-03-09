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

    Rectangle ScrollableView::GetVisibleRect() const
    {
        return { m_scrollOffset.X, m_scrollOffset.Y, m_viewportRect.Width, m_viewportRect.Height };
    }

    void ScrollableView::CalculateViewport()
    {
        m_viewportRect = m_fullViewSize.ToRectangle();
        
        m_viewportRect.Width = std::max<int>(0, static_cast<int>(m_viewportRect.Width));
        m_viewportRect.Height = std::max<int>(0, static_cast<int>(m_viewportRect.Height));

        auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);

        m_needVerticalScroll = m_contentSize.Height > m_viewportRect.Height;
        if (m_needVerticalScroll)
        {
            m_viewportRect.Width = std::max<int>(0, static_cast<int>(m_viewportRect.Width) - static_cast<int>(scrollSize));
        }

        m_needHorizontalScroll = m_contentSize.Width > m_viewportRect.Width;
        if (m_needHorizontalScroll)
        {
            m_viewportRect.Height = std::max<int>(0, static_cast<int>(m_viewportRect.Height) - static_cast<int>(scrollSize));
            
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

    void ScrollableView::SetScrollOffsetX(int offsetX)
    {
        if (!m_scrollBarHoriz || m_scrollOffset.X == offsetX)
            return;
        
        m_scrollBarHoriz->SetValue(offsetX);
    }

    void ScrollableView::SetScrollOffsetY(int offsetY)
    {
        if (!m_scrollBarVert || m_scrollOffset.Y == offsetY)
            return;
        
        m_scrollBarVert->SetValue(offsetY);
    }

    void ScrollableView::UpdateScrollBars()
    {
        bool needNotifyChange = false;
        if (!m_needVerticalScroll && m_scrollBarVert)
        {
            m_scrollBarVert.reset();
            m_scrollOffset.Y = 0;
            needNotifyChange = true;
        }
        else if (m_needVerticalScroll)
        {
            UpdateVerticalScrollBar();
        }

        if (!m_needHorizontalScroll && m_scrollBarHoriz)
        {
            m_scrollBarHoriz.reset();
            m_scrollOffset.X = 0;
            needNotifyChange = true;
            NotifyChange();
        }
        else if (m_needHorizontalScroll)
        {
            UpdateHorizontalScrollBar();
        }
        
        if (needNotifyChange)
        {
            NotifyChange();
        }
    }

    void ScrollableView::UpdateVerticalScrollBar()
    {
        auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
        Rectangle scrollRect
        {
            static_cast<int>(m_fullViewSize.Width - scrollSize) - 1, 
            1, 
            scrollSize, 
            m_fullViewSize.Height - 2u 
        };

        if (m_needHorizontalScroll)
        {
            scrollRect.Height -= scrollSize;
        }
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
        
        int clampedVal = std::clamp(m_scrollOffset.Y, 0, m_scrollBarVert->GetMax());
        if (clampedVal != m_scrollOffset.Y)
        {
            m_scrollBarVert->SetValue(clampedVal);
        }
    }

    void ScrollableView::UpdateHorizontalScrollBar()
    {
        auto scrollSize = m_owner->ToScale(m_owner->Appearance->ScrollBarSize);
        Rectangle scrollRect
        {
            1, 
            static_cast<int>(m_fullViewSize.Height - scrollSize) - 1, 
            m_fullViewSize.Width - 2u, 
            scrollSize
        };

        if (m_needVerticalScroll)
        {
            scrollRect.Width -= scrollSize;
        }
        if (!m_scrollBarHoriz)
        {
            m_scrollBarHoriz = std::make_unique<ScrollBar>(m_owner, false, scrollRect, false);
            m_scrollBarHoriz->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
            {
                m_scrollOffset.X = args.Value;
                NotifyChange();
            });
        }
        else
        {
            GUI::MoveWindow(m_scrollBarHoriz->Handle(), scrollRect);
        }

        m_scrollBarHoriz->SetMinMax(0, static_cast<int>(m_contentSize.Width - m_viewportRect.Width));
        m_scrollBarHoriz->SetPageStepValue(m_viewportRect.Width);
        m_scrollBarHoriz->SetStepValue(m_scrollStep.Width);
        
        int clampedVal = std::clamp(m_scrollOffset.X, 0, m_scrollBarHoriz->GetMax());
        if (clampedVal != m_scrollOffset.X)
        {
            m_scrollBarHoriz->SetValue(clampedVal);
        }
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

    bool ScrollableView::EnsureVisibility(const Rectangle& targetBounds) const
    {
        bool changed = false;

        if (m_scrollBarVert)
        {
            int viewTop = m_scrollOffset.Y + m_viewPadding.Top;
            int viewBottom = m_scrollOffset.Y + static_cast<int>(m_viewportRect.Height) - m_viewPadding.Bottom;
            int newY = m_scrollOffset.Y;

            if (targetBounds.Y < viewTop)
            {
                newY = targetBounds.Y - m_viewPadding.Top;
            }
            else if (targetBounds.Y + static_cast<int>(targetBounds.Height) > viewBottom)
            {
                newY = targetBounds.Y + static_cast<int>(targetBounds.Height) - static_cast<int>(m_viewportRect.Height) + m_viewPadding.Bottom;
            }
            
            if (newY != m_scrollOffset.Y)
            {
                m_scrollBarVert->SetValue(newY);
                changed = true;
            }
        }

        if (m_scrollBarHoriz)
        {
            int viewLeft = m_scrollOffset.X + m_viewPadding.Left;
            int viewRight = m_scrollOffset.X + static_cast<int>(m_viewportRect.Width) - m_viewPadding.Right;
            int newX = m_scrollOffset.X;

            if (targetBounds.X < viewLeft)
            {
                newX = targetBounds.X - m_viewPadding.Left;
            }
            else if (targetBounds.X + static_cast<int>(targetBounds.Width) > viewRight)
            {
                newX = targetBounds.X + static_cast<int>(targetBounds.Width) - static_cast<int>(m_viewportRect.Width) + m_viewPadding.Right;
            }
            
            if (newX != m_scrollOffset.X)
            {
                m_scrollBarHoriz->SetValue(newX);
                changed = true;
            }
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
