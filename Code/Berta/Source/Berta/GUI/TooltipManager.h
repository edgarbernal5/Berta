/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TOOLTIP_MANAGER_HEADER
#define BT_TOOLTIP_MANAGER_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Timer.h"

namespace Berta
{
    class ControlBase;
    class TooltipForm;

    class TooltipManager
    {
    public:
        TooltipManager();
        ~TooltipManager();
        
        void OnMouseMove(Window* window, const Point& globalPos, const std::wstring& specificText = L"", void* subItemId = nullptr);
        void OnMouseLeave();
        void OnControlDestroyed(ControlBase* control);
        void HideTooltip();
        
    private:
        void ShowTooltip(const std::wstring& text, Point position);
        
        Timer m_hoverTimer;
        Window* m_hoveredWindow{ nullptr };
        void* m_subItemId{ nullptr };
        std::wstring m_currentText;
        Point m_lastGlobalPos;
        std::unique_ptr<TooltipForm> m_tooltipForm{ nullptr };
    };
}

#endif