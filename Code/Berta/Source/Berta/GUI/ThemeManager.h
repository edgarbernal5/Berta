#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>

#include "EnumTypes.h"

namespace Berta
{
    struct ControlAppearance;

    class ThemeManager
    {
    public:
        static ThemeManager& Instance()
        {
            static ThemeManager instance;
            return instance;
        }
        
        void SetPalette(const ThemePalette& newPalette);
        
        template <typename TAppearance>
        std::shared_ptr<TAppearance> GetDefaultStyle()
        {
            auto typeId = std::type_index(typeid(TAppearance));
            
            if (m_defaultStyles.find(typeId) == m_defaultStyles.end())
            {
                auto newStyle = std::make_shared<TAppearance>();
                //newStyle->ApplyTheme(m_activePalette);
                m_defaultStyles[typeId] = newStyle;
            }
            
            return std::static_pointer_cast<TAppearance>(m_defaultStyles[typeId]);
        }
    private:
        
        ThemeManager() = default;
        
        ThemePalette m_activePalette;
        std::unordered_map<std::type_index, std::shared_ptr<ControlAppearance>> m_defaultStyles;
    };

}
