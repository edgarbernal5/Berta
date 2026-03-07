/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CONTROL_CLIPPING_HEADER
#define BT_CONTROL_CLIPPING_HEADER

#include "Berta/Paint/Graphics.h"

namespace Berta
{
    class ControlClipping
    {
    public:
        ControlClipping(Graphics& graphics, const Rectangle& clipRect);
        ~ControlClipping();
        
    private:
        Graphics& m_graphics;
    };
}

#endif
