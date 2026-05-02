/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_APPEARANCE_HEADER
#define BT_PROPERTY_GRID_APPEARANCE_HEADER

#include "Berta/GUI/ControlAppearance.h"

namespace Berta::Internal::PropertyGrid
{
    struct Appearance : public ControlAppearance
    {
        uint32_t CategoryHeight = 22u;
        uint32_t ExpanderButtonSize = 12u;
			
        Color HoverBackgroundColor{ 0xFFDDE6EB };
        Color SelectedBackgroundColor{ 0, 112, 192, 255 };
        Color SelectedTextColor{ 255, 255, 255, 255 };
        Color NormalTextColor{ 200, 200, 200, 255 };
    };
    
    using LayoutConfig = Internal::PropertyGrid::Appearance;
}

#endif
