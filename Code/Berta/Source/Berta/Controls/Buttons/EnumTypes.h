/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_BUTTONS_ENUM_TYPES_HEADER
#define BT_BUTTONS_ENUM_TYPES_HEADER

#include "Berta/GUI/ControlAppearance.h"

namespace Berta::Internal::Buttons
{
    struct Appearance : public ControlAppearance
    {
        Color HoverBackground;
        Color PressedBackground;
        Color DisabledBackground;
        
        uint32_t IconTextSpacing{ 4 };
			
        /*void ApplyTheme(const ThemePalette& theme) override
        {
            ControlAppearance::ApplyTheme(theme); // Llama a la base
            
            // Especialización del botón
            Background = theme.BaseBackground; // O el color que desees para botones
            HoverBackground = theme.BaseBackground.Lighten(0.2f);
            PressedBackground = theme.BaseBackground.Darken(0.2f);
            DisabledBackground = theme.BaseBackground.Lighten(0.5f);
        }*/
    };
}

#endif
