/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TooltipManager.h"

#include "Window.h"
#include "Berta/Controls/Tooltips/TooltipForm.h"

namespace Berta
{
    TooltipManager::TooltipManager()
    {
        m_hoverTimer.SetInterval(500);
        m_hoverTimer.Connect([this](const ArgTimer& args) 
        {
            m_hoverTimer.Stop();
            if (m_hoveredWindow && !m_currentText.empty())
            {
                ShowTooltip(m_currentText, m_lastGlobalPos);
            }
        });
    }

    TooltipManager::~TooltipManager()
    {
    }

    void TooltipManager::OnMouseMove(Window* window, const Point& globalPos, const std::wstring& specificText, void* subItemId)
    {
        if (!window || window->Flags.IsDisposed)
        {
            return;
        }
        
        if (m_hoveredWindow != window || m_subItemId != subItemId)
        {
            HideTooltip();
            m_hoveredWindow = window;
            m_subItemId = subItemId;
            
            m_currentText = specificText; 
                
            m_hoverTimer.Stop();
            if (!m_currentText.empty())
            {
                m_hoverTimer.SetOwner(m_hoveredWindow);
                m_hoverTimer.Start();
            }
        }
            
        m_lastGlobalPos = globalPos;
    }

    void TooltipManager::OnMouseLeave()
    {
        m_hoverTimer.Stop();
        HideTooltip();
        m_hoveredWindow = nullptr;
        m_subItemId = nullptr;
        m_currentText = L"";
    }

    void TooltipManager::OnControlDestroyed(ControlBase* control)
    {
        if (m_hoveredWindow && m_hoveredWindow->ControlWindowPtr && m_hoveredWindow->ControlWindowPtr->ControlPtr() == control)
        {
            OnMouseLeave();
        }
    }

    void TooltipManager::HideTooltip()
    {
        if (m_tooltipForm)
        {
            m_tooltipForm->Dispose();
            m_tooltipForm.reset();
        }
    }

    void TooltipManager::ShowTooltip(const std::wstring& text, Point position)
    {
        if (!m_hoveredWindow)
        {
            return;
        }
        
        auto& graphics = m_hoveredWindow->Renderer.GetGraphics();
        Size textSize = graphics.GetTextExtent(text);

        uint32_t paddingX = m_hoveredWindow->ToScale(8u);
        uint32_t paddingY = m_hoveredWindow->ToScale(4u);
        int positionOffsetX = m_hoveredWindow->ToScale(3);
        int positionOffsetY = m_hoveredWindow->ToScale(12);
        Size tooltipSize{ textSize.Width + paddingX, textSize.Height + paddingY };

        Point desiredPos = { position.X + positionOffsetX, position.Y + positionOffsetY };
    
        Rectangle workArea = API::GetMonitorWorkArea(desiredPos);
        if (workArea.Width > 0 && workArea.Height > 0)
        {
            int rightBound = workArea.X + static_cast<int>(workArea.Width);
            int bottomBound = workArea.Y + static_cast<int>(workArea.Height);

            if (desiredPos.X + static_cast<int>(tooltipSize.Width) > rightBound)
            {
                desiredPos.X = rightBound - static_cast<int>(tooltipSize.Width);
            }

            if (desiredPos.Y + static_cast<int>(tooltipSize.Height) > bottomBound)
            {
                desiredPos.Y = position.Y - static_cast<int>(tooltipSize.Height) - (positionOffsetY / 2);
            }
        }

        Rectangle tooltipRect{ desiredPos.X, desiredPos.Y, tooltipSize.Width, tooltipSize.Height };
    
        m_tooltipForm = std::make_unique<TooltipForm>(m_hoveredWindow, tooltipRect, FormStyle::Float(false));
        m_tooltipForm->SetCaption(text);
        m_tooltipForm->Show();
    }

}
