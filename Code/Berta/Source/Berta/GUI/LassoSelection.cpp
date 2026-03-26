/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "LassoSelection.h"

#include <algorithm>

namespace Berta
{
    void LassoSelection::Start(const Point& startPosition)
    {
        m_isActive = true;
        m_startPos = startPosition;
        m_currentPos = startPosition;
    }

    bool LassoSelection::Update(const Point& currentPosition)
    {
        if (!m_isActive || m_currentPos == currentPosition)
        {
            return false;
        }

        m_currentPos = currentPosition;
        return true;
    }

    void LassoSelection::End()
    {
        m_isActive = false;
    }

    Rectangle LassoSelection::GetRect() const
    {
        if (!m_isActive)
        {
            return { 0, 0, 0, 0 };
        }
        
        int x = std::min<int>(m_startPos.X, m_currentPos.X);
        int y = std::min<int>(m_startPos.Y, m_currentPos.Y);
        uint32_t width = static_cast<uint32_t>(std::max<int>(m_startPos.X, m_currentPos.X) - x);
        uint32_t height = static_cast<uint32_t>(std::max<int>(m_startPos.Y, m_currentPos.Y) - y);

        return { x, y, width, height };
    }

    void LassoSelection::Draw(Graphics& graphics, Graphics& selectionBox, const Color& blendColor, const Color& borderColor) const
    {
        if (!m_isActive)
        {
            return;
        }
        Rectangle rect = GetRect();
        
        if (rect.Width > 0 && rect.Height > 0)
        {
            Rectangle selectionBoxRect = rect;
            selectionBoxRect.X = selectionBoxRect.Y = 0;
            
            selectionBox.Begin();
            selectionBox.FillRectangle(selectionBoxRect, blendColor);
            selectionBox.DrawRectangle(selectionBoxRect, borderColor);
            selectionBox.Flush();

            graphics.Blend(rect, selectionBox, { 0,0 }, 0.5);
        }
    }
}