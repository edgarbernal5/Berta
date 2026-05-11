/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_TYPES_HEADER
#define BT_PROPERTY_GRID_TYPES_HEADER

#include <optional>

namespace Berta
{
    struct OptionalVector3 
    {
        std::optional<float> x;
        std::optional<float> y;
        std::optional<float> z;
    };
}

#endif