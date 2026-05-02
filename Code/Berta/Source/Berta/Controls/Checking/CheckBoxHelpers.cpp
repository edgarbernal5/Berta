/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "CheckBoxHelpers.h"

namespace Berta::Internal::CheckBox
{
    void DrawCheckmark(Graphics& graphics, const Point& position, int size, Color color, float strokeWidth)
    {
        Point p1 = { 
            position.X + static_cast<int>(size * 0.2f), 
            position.Y + static_cast<int>(size * 0.5f) 
        };
        
        Point p2 = { 
            position.X + static_cast<int>(size * 0.45f), 
            position.Y + static_cast<int>(size * 0.75f) 
        };
        
        Point p3 = { 
            position.X + static_cast<int>(size * 0.8f), 
            position.Y + static_cast<int>(size * 0.25f) 
        };

        graphics.DrawLine(p1, p2, strokeWidth, color);
        graphics.DrawLine(p2, p3, strokeWidth, color);
        
        graphics.DrawLine({ p1.X, p1.Y + 1 }, { p2.X, p2.Y + 1 }, strokeWidth, color);
        graphics.DrawLine({ p2.X, p2.Y + 1 }, { p3.X, p3.Y + 1 }, strokeWidth, color);
    }
}
