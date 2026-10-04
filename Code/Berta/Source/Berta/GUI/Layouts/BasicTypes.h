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
            OpenBracket,
            CloseBracket,
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
            
            Margin,
            Padding,

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
    
    struct Thickness
    {
        double Left = 0.0;
        double Top = 0.0;
        double Right = 0.0;
        double Bottom = 0.0;

        Thickness() = default;
        Thickness(double uniform) 
            : Left(uniform), Top(uniform), Right(uniform), Bottom(uniform) {}
        Thickness(double horizontal, double vertical) 
            : Left(horizontal), Top(vertical), Right(horizontal), Bottom(vertical) {}
        Thickness(double left, double top, double right, double bottom) 
            : Left(left), Top(top), Right(right), Bottom(bottom) {}
    };

    struct Dimension
    {
        double value = 0.0;
        DimensionUnit unit = DimensionUnit::Pixels;

        [[nodiscard]] bool IsPercentage() const noexcept { return unit == DimensionUnit::Percentage; }
    };
    using PropertyValue = std::variant<int, double, Dimension, Thickness, std::string>;
    
    // Helper clásico para usar con std::visit (Pattern Matching de tipos)
    template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };
    template<class... Ts> Overload(Ts...) -> Overload<Ts...>;
}

#endif