/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SCOPED_CLIP_HEADER
#define BT_SCOPED_CLIP_HEADER

#include "Berta/Paint/Graphics.h"

namespace Berta
{
    struct ScopedClip
    {
        ScopedClip(Graphics& graphics, const Rectangle& clipArea) :
            m_graphics(graphics)
        {
            m_graphics.SetClipping(clipArea);
        }
        
        ~ScopedClip()
        {
            m_graphics.EndClipping();
        }
        
    private:
        Graphics& m_graphics;
    };
}

#endif
