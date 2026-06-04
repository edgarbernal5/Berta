/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_BASIC_TYPES_HEADER
#define BT_LAYOUT_BASIC_TYPES_HEADER

#include <variant>
#include <string_view>

namespace Berta
{
    struct Token
    {
        enum class Type
        {
            None,

            Identifier,
            NumberInt,
            NumberDouble,
            Percentage,
            String,
            OpenBrace,
            CloseBrace,
            Equal,
            Splitter,
            Colon,
            Comma,
            Unknown,
            EndOfStream,

            VerticalLayout = 256,
            HorizontalLayout,
            Width,
            Height,

            MinHeight,
            MaxHeight,
            MinWidth,
            MaxWidth,

            Dock,
            DockPane
        };

        Type type;
        std::string_view value;
        //size_t line;
        //size_t column;
    };
    
    struct Number
    {
        double value = 0.0;
        bool isPercentage = false;
    
        void SetValue(int v) { value = static_cast<double>(v); }
        void SetValue(double v) { value = v; }
    };
    
    enum class DimensionUnit : uint8_t
    {
        Pixels,
        Percentage
    };

    struct Dimension
    {
        double value = 0.0;
        DimensionUnit unit = DimensionUnit::Pixels;

        [[nodiscard]] bool IsPercentage() const noexcept { return unit == DimensionUnit::Percentage; }
    };
    using PropertyValue = std::variant<int, double, Dimension, std::string>;
    
    template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };
    template<class... Ts> Overload(Ts...) -> Overload<Ts...>;
}

#endif