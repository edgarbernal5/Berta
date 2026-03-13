/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ControlClipping.h"

namespace Berta
{
    ControlClipping::ControlClipping(Graphics& graphics) : 
        m_graphics(graphics)
    {
    }

    ControlClipping::ControlClipping(Graphics& graphics, const Rectangle& clipRect) :
        m_graphics(graphics)
    {
        SetClipping(clipRect);
    }

    void ControlClipping::SetClipping(const Rectangle& clipRect)
    {
        m_isClipping = true;
        m_graphics.SetClipping(clipRect);
    }

    ControlClipping::~ControlClipping()
    {
        if (!m_isClipping)
            return;
        
        m_graphics.EndClipping();
    }
}
