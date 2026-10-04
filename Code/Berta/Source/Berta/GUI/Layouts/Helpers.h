/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_HELPERS_HEADER
#define BT_LAYOUT_HELPERS_HEADER

#include "BasicTypes.h"

namespace Berta
{
    class LayoutNode;
}

namespace Berta::Layouts
{
    Berta::Thickness GetScaledThickness(LayoutNode* node, const std::string& propName, float dpi);
}

#endif

