/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ControlClipping.h"

namespace Berta
{
    ControlClipping::ControlClipping(Graphics& graphics, const Rectangle& clipRect) :
        m_graphics(graphics)
    {
        m_graphics.SetClipping(clipRect);
    }

    ControlClipping::~ControlClipping()
    {
        m_graphics.EndClipping();
    }
}
