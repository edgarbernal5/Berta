/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COLOR_PICKER_HEADER
#define BT_COLOR_PICKER_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include <string>

namespace Berta
{
    namespace Internal::ColorPicker
    {        
        class Reactor : public ControlReactor
        {
        public:
            void Update(Graphics& graphics) override;

            void MouseEnter(Graphics& graphics, const ArgMouse& args) override;
            void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
            void MouseDown(Graphics& graphics, const ArgMouse& args) override;
            void MouseUp(Graphics& graphics, const ArgMouse& args) override;
			
            void SetColor(const Color& color)
            {
                m_colorSelected = color;
            }
            
            Color GetColor() const
            {
                return m_colorSelected;
            }
        private:
            enum class State : uint8_t
            {
                Normal,
                Pressed,
                Hovered
            };
            State m_status{ State::Normal };
            Color m_colorSelected;
        };
    }
	
    class ColorPicker : public Control<Category::ControlTag, Internal::ColorPicker::Reactor>
    {
    public:
        ColorPicker() = default;
        ColorPicker(Window* parent, const Rectangle& rectangle, const Color& color);
        
        Color GetColor() const;
        void SetColor(const Color& color);
    };
}

#endif