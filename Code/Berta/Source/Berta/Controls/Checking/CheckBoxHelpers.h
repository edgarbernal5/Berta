/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_HEADER
#define BT_PROPERTY_GRID_HEADER

#include "Berta/Paint/Graphics.h"

namespace Berta::Internal::CheckBox
{
    void DrawCheckmark(Graphics& graphics, const Point& position, int size, Color color, float strokeWidth = 1.0f);
}
#endif