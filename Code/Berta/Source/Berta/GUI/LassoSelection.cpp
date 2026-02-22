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
        return true; // Hubo un cambio, el control padre debería repintarse
    }

    void LassoSelection::End()
    {
        m_isActive = false;
    }

    Rectangle LassoSelection::GetRect() const
    {
        if (!m_isActive) return { 0, 0, 0, 0 };

        // Normalizamos el rectángulo por si el usuario arrastra hacia arriba/izquierda
        int x = (std::min)(m_startPos.X, m_currentPos.X);
        int y = (std::min)(m_startPos.Y, m_currentPos.Y);
        uint32_t width = static_cast<uint32_t>((std::max)(m_startPos.X, m_currentPos.X) - x);
        uint32_t height = static_cast<uint32_t>((std::max)(m_startPos.Y, m_currentPos.Y) - y);

        return { x, y, width, height };
    }

    void LassoSelection::Draw(Graphics& graphics, const Color& fillColor, const Color& borderColor) const
    {
        if (!m_isActive) return;

        Rectangle rect = GetRect();
        
        // Evitar dibujar rectángulos con área cero
        if (rect.Width > 0 && rect.Height > 0)
        {
            graphics.FillRectangle(rect, fillColor);
            graphics.DrawRectangle(rect, borderColor);
        }
    }
}